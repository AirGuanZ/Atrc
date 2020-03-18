#include <QFileDialog>
#include <QMessageBox>

#include <agz/editor/imexport/json_export_context.h>
#include <agz/editor/imexport/json_export_dialog.h>
#include <agz/editor/renderer/renderer_widget.h>
#include <agz/editor/scene/scene_mgr.h>
#include <agz/editor/ui/global_setting_widget.h>
#include <agz/factory/factory.h>

AGZ_EDITOR_BEGIN

void export_json(
    SceneManager        *scene_mgr,
    ObjectContext       *obj_ctx,
    EnvirLightSlot      *envir_light,
    PreviewWindow       *preview_window,
    GlobalSettingWidget *global_settings,
    RendererPanel       *renderer_panel)
{
    try
    {
        const QString scene_desc_filename = QFileDialog::getSaveFileName(
            nullptr, QString(), QString(), "JSON (*.json)");
        if(scene_desc_filename.isEmpty())
            return;
        JSONExportContext ctx(scene_desc_filename.toStdString());

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

            auto filter_grp = newRC<tracer::ConfigGroup>();
            filter_grp->insert_str("type", "box");
            filter_grp->insert_real("radius", real(0.5));
            setting_config->insert_child("film_filter", filter_grp);

            setting_config->insert_real(
                "eps", real(global_settings->scene_eps->value()));

            auto post_processor_arr = newRC<tracer::ConfigArray>();
            auto save_img_grp = newRC<tracer::ConfigGroup>();
            save_img_grp->insert_str("type", "save_to_img");
            save_img_grp->insert_str("filename", "${scene-directory}/output.png");
            save_img_grp->insert_real("inv_gamma", real(2.2));

            post_processor_arr->push_back(save_img_grp);
            setting_config->insert_child("post_processors", post_processor_arr);

            root_grp.insert_child("rendering", setting_config);
        }

        auto json = tracer::factory::config_to_json(root_grp);
        std::ofstream fout(
            scene_desc_filename.toStdString(), std::ios::out | std::ios::trunc);
        if(!fout)
        {
            throw std::runtime_error(
                "failed to open file " + scene_desc_filename.toStdString());
        }
        
        fout << json.dump(4) << std::endl;
    }
    catch(const std::exception &e)
    {
        QMessageBox::information(nullptr, "Error", e.what());
    }
}

AGZ_EDITOR_END
