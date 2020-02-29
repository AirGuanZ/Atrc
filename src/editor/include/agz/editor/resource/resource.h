#pragma once

/*
Ownership:
    Other ResourceWidget
      ==> ResourceSlot
            ==> ResourceReference
                  --> ResourceInPool
                        ==>
            ==============> ResourcePanel
                              ==> ResourceWidget

Each object has a 'dirty' flag incidcating whether the corresponding tracer object need to be updated.
When a widget is edited, its dirty flag is set to true and propagated to all its ancestors, and the
first affected root object will emit a message to editor. The editor will then frozen all accesses to
the editor object DAG (including the rendering process), and call the updating method of each root object
to refresh the tracer object DAG. All objecs with 'true' dirty flag must update their corresponding tracer objects.
*/

#include <agz/editor/resource/resource_slot.h>
#include <agz/editor/resource/object_context.h>

#include <agz/editor/resource/resource_widget.inl>
#include <agz/editor/resource/resource_panel.inl>
#include <agz/editor/resource/resource_pool.inl>
#include <agz/editor/resource/resource_slot.inl>
