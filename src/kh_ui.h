#if !defined(KH_UI)
#if 0
#define USE_KH_IMGUI

#if defined(USE_KH_IMGUI)

#define UI_MAX_ITEM_PER_WINDOW 64
#define UI_MAX_ITEMS 4096
#define UI_MAX_WINDOWS 64

enum kh_ui_interaction_type
{
    UIInteraction_move,
    UIInteraction_setptr,
    UIInteraction_expand,
};

enum kh_ui_element_type
{
    UIElementType_b32,
    UIElementType_f32,
    UIElementType_v2,
    UIElementType_v3,
};

enum kh_mouse_click
{
    UIClick_left,
    UIClick_right,
    UIClick_middle,
    UIClick_b4,
    UIClick_b5,

    UIClick_count,
};

enum kh_ui_draw_type
{
    UIDraw_texture,
    UIDraw_string,
    UIDraw_background,
};

typedef struct kh_ui_element
{
    union
    {
        b32 val_b32;
        f32 val_f32;
        v2 val_v2;
        v3 val_v3;
    };
}kh_ui_element;

typedef struct kh_ui_item_id
{
    u32 index;
}kh_ui_item_id;

typedef struct kh_ui_interaction
{
    u32 id;
    kh_ui_interaction_type type;

    void *target;
    // NOTE(flo): new value for target
    union
    {
        void *generic;
        struct kh_ui_window *owner;
    };
}kh_ui_interaction;

typedef struct kh_ui_draw
{
    kh_ui_draw_type type;
    void *data;
}kh_ui_draw;

typedef struct kh_ui_item
{
    kh_ui_item_id id;
    kh_ui_item *next;
    kh_ui_interaction interaction;
    kh_ui_draw draw;
    v2 pos;
    v2 size;
    u32 pad_0;
}kh_ui_item;

typedef struct kh_ui_window
{
    kh_ui_item_id id;
    kh_ui_item sentinel;
}kh_ui_window;

typedef struct kh_ui_state
{
    // TODO(flo): better way for not adding every windows/items each frame
    b32 init;

    // TODO(flo): do we need a hash here!
    u32 item_count;
    kh_ui_item items[UI_MAX_ITEMS];

    u32 wnd_count;
    kh_ui_window wnds[UI_MAX_WINDOWS];

    kh_ui_item_id next_hot;
    kh_ui_item_id hot;
    kh_ui_item_id active;

    v2 mouse_p;
    b32 mouse[UIClick_count];
    b32 mouse_down[UIClick_count];
    b32 mouse_up[UIClick_count];

    font_id ft;

    stack_allocator stack;


    // TODO(flo): remove this once the ui api has its own rendering
    render_group *rd;
}kh_ui_state;

KH_INLINE kh_ui_item *
add_ui_item(kh_ui_state *uistate)
{
    kh_assert(uistate->item_count < array_count(uistate->items));
    kh_ui_item *res = uistate->items + uistate->item_count++;
    res->id.index = uistate->item_count - 1;
    return(res);
}

KH_INLINE kh_ui_item *
get_ui_item(kh_ui_state *uistate, kh_ui_item_id id)
{
    kh_assert(uistate->item_count < array_count(uistate->items));
    kh_ui_item *res = uistate->items + id.index;
    return(res);
}

KH_INTERN void
add_item_to_wnd(kh_ui_state *uistate, kh_ui_item *item)
{
    kh_ui_window *wnd = uistate->wnds + (uistate->wnd_count - 1);
    if(!wnd->sentinel.next)
    {
        wnd->sentinel.next = item;
        item->next = &wnd->sentinel;
    }
    else
    {
        item->next = wnd->sentinel.next;
        wnd->sentinel.next = item;
    }
}

KH_INTERN void
kh_add_window(kh_ui_state *uistate, v2 pos, v2 size)
{
    kh_ui_item *item = add_ui_item(uistate);
    kh_ui_window *wnd = uistate->wnds + uistate->wnd_count++;
    if(!uistate->init)
    {
        item->interaction.type = UIInteraction_move;
        item->pos = pos;
        item->size = size;
        item->draw.type = UIDraw_background;
        v4 color = { 0, 0, 0, 0.3f};
        item->draw.data = kh_push_struct(&uistate->stack, v4);
        v4 *data = (v4 *)item->draw.data;
        *data = color;
        wnd->sentinel = {};
        wnd->id.index = item->id.index;
    }
    // add_quad_color_entry(uistate->rd, identity_matrix(), V3(pos, 0), V3(pos.x + size.x, pos.y, 0), V3(pos.x, pos.y + size.y, 0), V3(pos + size, 0), *(v4 *)item->draw.data, RenderEntry_noprojection);
}

KH_INTERN b32
ui_button(kh_ui_state *uistate, v2 pos, v2 size, kh_ui_interaction interact, kh_ui_draw draw)
{
    kh_ui_item *item = add_ui_item(uistate);
    if(!uistate->init)
    {
        add_item_to_wnd(uistate, item);
        item->pos = pos;
        item->size = size;
        item->interaction = interact;
        item->draw = draw;
    }
    // TODO(flo): frame interaction

    return(false);
}

KH_INTERN void
kh_ui_init(kh_ui_state *uistate, font_id ft)
{
    uistate->stack = {};


    uistate->next_hot = {};
    uistate->hot = {};
    uistate->active = {};

    uistate->ft = ft;
    uistate->init = false;
}

KH_INTERN void
kh_ui_begin(kh_ui_state *uistate, v2 mouse_p, render_group *rd)
{
    uistate->item_count = 1;
    uistate->wnd_count = 1;
    uistate->mouse_p = mouse_p;
    uistate->rd = rd;
}

KH_INTERN void
kh_ui_end(kh_ui_state *uistate)
{
    for(u32 wnd_i = 1; wnd_i < uistate->wnd_count; ++wnd_i)
    {
        kh_ui_window *wnd = uistate->wnds + wnd_i;
        kh_ui_item *item = uistate->items + wnd->id.index;
        if(test_point_rect(uistate->mouse_p, create_rect(item->pos, item->size)))
        {
            // NOTE(flo): preparation for next frame interaction
        }
    }

    uistate->init = true;
}

KH_INTERN void
kh_ui_render()
{

}

#if 0
KH_INLINE b32
ui_item_is_wnd(kh_ui_state *uistate, kh_ui_item_id id)
{
    kh_ui_item *item = get_ui_item(uistate, id);
    b32 res = (id.index == item->owner_id.index);
    return(res);
}
    // uistate->sen_wnd = {};
    // uistate->cur_wnd = uistate->sen_wnd.next;

  if(!uistate->sen_wnd.next)
    {
        kh_ui_window *new_wnd = kh_pack_struct(uistate->block, kh_ui_window);
        uistate->sen_wnd.next = new_wnd;
        new_wnd->next = &uistate->sen_wnd;
        uistate->cur_wnd = new_wnd;
    }
    else if(!uistate->cur_wnd)
    {
        kh_ui_window *new_wnd = kh_pack_struct(uistate->block, kh_ui_window);
        new_wnd->next = uistate->sen_wnd.next;
        uistate->sen_wnd.next = new_wnd;
    }

#endif

#elif defined(USE_KH_RMGUI)

/* 
    struct kh_ui_id
    {
        u32 parent;
        u32 index;
    };

    struct kh_ui_button
    {
        char *txt;
        v2 size;
    };

    create instances
    add handlers with callback queue system
    update (set, get, delete, create)

    draw


*/
#endif
#endif //USE_GUI

#define KH_UI
#endif //KH_UI