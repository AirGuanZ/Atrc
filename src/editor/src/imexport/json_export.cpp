#include <QFileDialog>
#include <QMessageBox>

#include <agz/editor/imexport/json_export.h>
#include <agz/editor/imexport/json_export_context.h>
#include <agz/editor/post_processor/post_processor_seq.h>
#include <agz/editor/renderer/renderer_widget.h>
#include <agz/editor/scene/scene_mgr.h>
#include <agz/editor/ui/global_setting_widget.h>
#include <agz/factory/factory.h>

AGZ_EDITOR_BEGIN

bool export_json_to_file(
    const std::string &filename,
    SceneManager        *scene_mgr,
    ObjectContext       *obj_ctx,
    EnvirLightSlot      *envir_light,
    PostProcessorSeq    *post_processors,
    PreviewWindow       *preview_window,
    GlobalSettingWidget *global_settings,
    FilmFilterPanel     *film_filter,
    RendererPanel       *renderer_panel)
{
    try
    {
        JSONExportContext ctx(filename);

        tracer::ConfigGroup root_grp;

        // scene

        {
            auto scene_config   = newRC<tracer::ConfigGroup>();
            scene_config->insert_str("type", "default");

            scene_config->insert_child("entities", scene_mgr->to_config(ctx));

            obj_ctx->pool<tracer::Material> ()->to_config(*scene_config, ctx);
            obj_ctx->pool<tracer::Medium>   ()->to_config(*scene_config, ctx);
            obj_ctx->pool<tracer::Geometry> ()->to_config(*scene_config, ctx);
            obj_ctx->pool<tracer::Texture2D>()->to_config(*scene_config, ctx);
            obj_ctx->pool<tracer::Texture3D>()->to_config(*scene_config, ctx);

            auto env_grp = envir_light->to_config(ctx);
            if(env_grp)
                scene_config->insert_child("env", env_grp);

            auto aggregate_grp = newRC<tracer::ConfigGroup>();
            aggregate_grp->insert_str("type", "bvh");
            scene_config->insert_child("aggregate", aggregate_grp);

            root_grp.insert_child("scene", scene_config);
        }

        // rendering setting

        {
            auto setting_config = newRC<tracer::ConfigGroup>();

            auto camera_grp = preview_window->to_config();
            setting_config->insert_child("camera", camera_grp);

            auto renderer_grp = renderer_panel->to_config();
            setting_config->insert_child("renderer", renderer_grp);

            auto reporter_grp = newRC<tracer::ConfigGroup>();
            reporter_grp->insert_str("type", "stdout");
            setting_config->insert_child("reporter", reporter_grp);

            setting_config->insert_int(
                "width",  preview_window->get_camera_panel()
                                        ->get_export_frame_width());
            setting_config->insert_int(
                "height", preview_window->get_camera_panel()
                                        ->get_export_frame_height());

            auto filter_grp = film_filter->to_config();
            setting_config->insert_child("film_filter", filter_grp);

            setting_config->insert_real(
                "eps", real(global_settings->scene_eps->value()));

            auto post_processor_arr = post_processors->to_config();
            setting_config->insert_child("post_processors", post_processor_arr);

            root_grp.insert_child("rendering", setting_config);
        }

        auto json = tracer::factory::config_to_json(root_grp);
        std::ofstream fout(
            filename, std::ios::out | std::ios::trunc);
        if(!fout)
        {
            throw std::runtime_error(
                "failed to open file " + filename);
        }
        
        fout << json.dump(4) << std::endl;

    }
    catch(const std::exception &e)
    {
        QMessageBox::information(nullptr, "Error", e.what());
        return false;
    }

    return true;
}

void export_json(
    SceneManager        *scene_mgr,
    ObjectContext       *obj_ctx,
    EnvirLightSlot      *envir_light,
    PostProcessorSeq    *post_processors,
    PreviewWindow       *preview_window,
    GlobalSettingWidget *global_settings,
    FilmFilterPanel     *film_filter,
    RendererPanel       *renderer_panel)
{
    const QString scene_desc_filename = QFileDialog::getSaveFileName(
        nullptr, QString(), QString(), "JSON (*.json)");
    if(scene_desc_filename.isEmpty())
        return;

    export_json_to_file(
        scene_desc_filename.toStdString(),
        scene_mgr, obj_ctx, envir_light, post_processors,
        preview_window, global_settings, film_filter, renderer_panel);
}

AGZ_EDITOR_END
