static uint8_t image[280][280][3];
static void flush(lv_disp_drv_t *drv,const lv_area_t *area,lv_color_t *colors) {
    for(int y=area->y1;y<=area->y2;y++) for(int x=area->x1;x<=area->x2;x++) {
        lv_color32_t c; c.full=lv_color_to32(*colors++);
        image[y][x][0]=c.ch.red;image[y][x][1]=c.ch.green;image[y][x][2]=c.ch.blue;
    }
    lv_disp_flush_ready(drv);
}
static void snapshot(const char *name) {
    lv_obj_update_layout(lv_scr_act());
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL);
    FILE *file=fopen(name,"wb");assert(file);
    int w=lv_disp_get_hor_res(NULL),h=lv_disp_get_ver_res(NULL);fprintf(file,"P6\n%d %d\n255\n",w,h);for(int y=0;y<h;y++)fwrite(image[y],3,w,file);fclose(file);
}
int main(void) {
    lv_init();
    static lv_color_t pixels[240*280];
    static lv_disp_draw_buf_t buf;
    lv_disp_draw_buf_init(&buf,pixels,NULL,240*280);
    lv_disp_drv_t drv;lv_disp_drv_init(&drv);drv.hor_res=240;drv.ver_res=280;drv.draw_buf=&buf;drv.flush_cb=flush;drv.rotated=LV_DISP_ROT_270;
    lv_disp_drv_register(&drv);
    printf("Logical display: %d x %d\n",lv_disp_get_hor_res(NULL),lv_disp_get_ver_res(NULL));lv_obj_t *screen=zmk_display_status_screen();lv_scr_load(screen);lv_obj_update_layout(screen);
    lv_mem_monitor_t memory;lv_mem_monitor(&memory);printf("Graphics heap used: %lu bytes\n",(unsigned long)(memory.total_size-memory.free_size));
    assert(lv_obj_get_y(modifier_boxes[0])+lv_obj_get_height(modifier_boxes[0]) < lv_obj_get_y(battery_widget.obj));
    battery_bar_battery_update_cb((struct battery_update_state){0,82});
    battery_bar_battery_update_cb((struct battery_update_state){1,76});
    battery_bar_connection_update_cb((struct connection_update_state){0,true});
    battery_bar_connection_update_cb((struct connection_update_state){1,true});
    lv_timer_handler();lv_tick_inc(500);lv_timer_handler();lv_tick_inc(500);lv_timer_handler();
    assert(brightness==50);
    lv_obj_t *slot0 = lv_obj_get_child(battery_widget.obj, 0);
    lv_obj_t *slot1 = lv_obj_get_child(battery_widget.obj, 1);
    lv_obj_update_layout(screen);
    assert(lv_obj_get_x(slot1) < lv_obj_get_x(slot0));
    for (uint32_t pos=0;pos<35;pos++) {
        bool left = pos<30 ? pos%10<5 : pos<33;
        assert(left_slot_for_position(0,pos)==(left?0:1));
        assert(left_slot_for_position(1,pos)==(left?1:0));
    }
    assert(left_slot_for_position(255,0)==-1 && left_slot_for_position(0,35)==-1);
    peripheral_side_listener(&(struct zmk_position_state_changed){.source=0,.position=0,.state=true});
    lv_tick_inc(100);lv_timer_handler();lv_obj_update_layout(screen);
    assert(lv_obj_get_x(slot0)<lv_obj_get_x(slot1));
    peripheral_side_listener(&(struct zmk_position_state_changed){.source=1,.position=5,.state=true});
    assert(atomic_get(&left_peripheral_slot)==0);
    peripheral_side_listener(&(struct zmk_position_state_changed){.source=1,.position=30,.state=true});
    lv_tick_inc(100);lv_timer_handler();lv_obj_update_layout(screen);
    assert(lv_obj_get_x(slot1)<lv_obj_get_x(slot0));
    battery_bar_battery_update_cb((struct battery_update_state){0,44});
    assert(strcmp(lv_label_get_text(lv_obj_get_child(slot0,1)),"44")==0);
    assert(strcmp(lv_label_get_text(lv_obj_get_child(slot1,1)),"76")==0);
    battery_bar_battery_update_cb((struct battery_update_state){0,82});
    peripheral_side_listener(&(struct zmk_position_state_changed){.source=0,.position=0,.state=false});
    peripheral_side_listener(&(struct zmk_position_state_changed){.source=255,.position=0,.state=true});
    assert(atomic_get(&left_peripheral_slot)==1);

    fake_layer=2; fake_wpm=93; lv_tick_inc(100);lv_timer_handler();
    assert(strcmp(lv_label_get_text(value),"SYMBOLS")==0);
    assert(strcmp(lv_label_get_text(wpm_label),"93 WPM")==0);
    lv_tick_inc(200);lv_timer_handler();
    assert(animated_wpm > 720 && animated_wpm < 930);
    assert(lv_obj_get_style_bg_color(wpm_bar,LV_PART_INDICATOR).full==wpm_color(animated_wpm).full);
    assert(lv_obj_get_style_bg_opa(wpm_bar,LV_PART_INDICATOR)==LV_OPA_COVER);
    lv_tick_inc(300);lv_timer_handler();assert(animated_wpm == 930);
    fake_wpm=175;lv_tick_inc(100);lv_timer_handler();lv_tick_inc(500);lv_timer_handler();
    assert(lv_bar_get_value(wpm_bar)==1500);
    assert(strcmp(lv_label_get_text(wpm_label),"175 WPM")==0);
    assert(lv_obj_get_style_bg_color(wpm_bar,LV_PART_INDICATOR).full==lv_color_hex(0xffb14a).full);
    fake_wpm=0;lv_tick_inc(100);lv_timer_handler();lv_tick_inc(500);lv_timer_handler();
    assert(lv_bar_get_value(wpm_bar)==0);

    fake_layer=3;fake_layer_id=4;lv_tick_inc(100);lv_timer_handler();
    assert(strcmp(lv_label_get_text(value),"PRECISION")==0);
    fake_layer_name="Fine";lv_tick_inc(100);lv_timer_handler();
    assert(strcmp(lv_label_get_text(value),"FINE")==0);
    fake_layer_name=NULL;fake_layer_id=-1;
    fake_layer=0;fake_wpm=72;lv_tick_inc(100);lv_timer_handler();
    lv_tick_inc(500);lv_timer_handler();
    snapshot("chary-focused-normal.ppm");
    assert(strcmp(lv_label_get_text(context),"DPI 800 / 300")==0);
    assert(strcmp(lv_label_get_text(value),"QWERTY")==0);
    assert(strcmp(lv_label_get_text(wpm_label),"72 WPM")==0);
    fake_layer=4;fake_dpi=800;now=1000;charybdis_display_dpi_changed(false);
    assert(strcmp(lv_label_get_text(value),"800")==0);
    snapshot("chary-focused-dpi.ppm");
    assert(lv_obj_get_y(value)+lv_obj_get_height(value)<=lv_obj_get_y(context));
    now=3999;lv_tick_inc(2999);lv_timer_handler();assert(notice_visible);
    fake_dpi=1200;charybdis_display_dpi_changed(false);now=4001;lv_tick_inc(2);lv_timer_handler();assert(notice_visible);
    now=6999;lv_tick_inc(2998);lv_timer_handler();assert(!notice_visible);
    assert(strcmp(lv_label_get_text(value),"PRECISION")==0);
    assert(lv_obj_get_width(value)==248);
    snapshot("chary-focused-precision.ppm");
    assert(lv_obj_get_y(value)+lv_obj_get_height(value)<=lv_obj_get_y(context));
    assert(lv_obj_get_y(context)+lv_obj_get_height(context)<lv_obj_get_y(modifier_boxes[0]));
    assert(lv_obj_get_width(context)<=248);
    lv_point_t dpi_text_size;
    lv_txt_get_size(&dpi_text_size,"DPI 1600 / 400",&lv_font_montserrat_16,0,0,LV_COORD_MAX,LV_TEXT_FLAG_NONE);
    assert(dpi_text_size.x<=lv_obj_get_width(context));
    assert(lv_obj_get_x(cw_label)+lv_obj_get_width(cw_label)<lv_obj_get_x(context));
    caps_word_listener(&(struct zmk_caps_word_state_changed){true});fake_caps=0;fake_report.body.modifiers=0xf0;lv_tick_inc(100);lv_timer_handler();
    assert(last_cw==1 && last_caps==0 && last_mods==0xf0);
    assert(strcmp(lv_label_get_text(context),"DPI 1200 / 300")==0);
    fake_precision=400;charybdis_display_dpi_changed(true);
    assert(strcmp(lv_label_get_text(heading),"PRECISION DPI")==0);
    assert(strcmp(lv_label_get_text(value),"400")==0);
    now+=3100;lv_tick_inc(3100);lv_timer_handler();
    assert(strcmp(lv_label_get_text(context),"DPI 1200 / 400")==0);
    fake_brightness=75;charybdis_display_brightness_changed();
    assert(brightness==75 && strcmp(lv_label_get_text(value),"75%")==0);
    activity=1;activity_listener(NULL);assert(brightness==0);
    activity=0;activity_listener(NULL);assert(brightness==75);
    puts("Display states, three-second timer reset/expiry, modifier/lock state and idle backlight verified");
    return 0;
}
