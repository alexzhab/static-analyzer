/*****************************************************************************
 * gnome_callbacks.c : Callbacks for the Gnome plugin.
 *****************************************************************************
 * Copyright (C) 2000, 2001 VideoLAN
 * $Id: gnome_callbacks.c,v 1.29 2001/05/15 01:01:44 stef Exp $
 *
 * Authors: Samuel Hocevar <sam@zoy.org>
 *          St�phane Borel <stef@via.ecp.fr>
 *      
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

#define MODULE_NAME gnome
#include "modules_inner.h"

/*****************************************************************************
 * Preamble
 *****************************************************************************/
#include "defs.h"

#include <gnome.h>

#include "config.h"
#include "common.h"
#include "threads.h"
#include "mtime.h"

#include "stream_control.h"
#include "input_ext-intf.h"

#include "interface.h"
#include "intf_playlist.h"
#include "intf_msg.h"

#include "video.h"
#include "video_output.h"
#include "audio_output.h"

#include "gnome_callbacks.h"
#include "gnome_playlist.h"
#include "gnome_interface.h"
#include "gnome_support.h"
#include "intf_gnome.h"

#include "main.h"

/*****************************************************************************
 * Interface callbacks
 *****************************************************************************
 * The following callbacks are related to the main interface window.
 *****************************************************************************/
void
on_intf_window_destroy                 (GtkObject       *object,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(object), "intf_window" );

    vlc_mutex_lock( &p_intf->change_lock );
    p_intf->b_die = 1;
    vlc_mutex_unlock( &p_intf->change_lock );
}


gboolean
on_slider_button_press_event           (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(widget), "intf_window" );

    vlc_mutex_lock( &p_intf->change_lock );
    p_intf->p_sys->b_slider_free = 0;
    vlc_mutex_unlock( &p_intf->change_lock );

    return FALSE;
}


gboolean
on_slider_button_release_event         (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(widget), "intf_window" );

    vlc_mutex_lock( &p_intf->change_lock );
    p_intf->p_sys->b_slider_free = 1;
    vlc_mutex_unlock( &p_intf->change_lock );

    return FALSE;
}


void
on_intf_window_drag_data_received      (GtkWidget       *widget,
                                        GdkDragContext  *drag_context,
                                        gint             x,
                                        gint             y,
                                        GtkSelectionData *data,
                                        guint            info,
                                        guint            time,
                                        gpointer         user_data)
{
    intf_thread_t * p_intf = GetIntf( GTK_WIDGET( widget ), "intf_window" );
    int             i_end  = p_main->p_playlist->i_size;

    GnomeDropDataReceived( p_intf, data, info, PLAYLIST_END );

     if( p_intf->p_input != NULL )
     {
        /* FIXME: temporary hack */
        p_intf->p_input->b_eof = 1;
     }
     
    intf_PlaylistJumpto( p_main->p_playlist, i_end - 1 );
    p_main->p_playlist->b_stopped = 0;
}


void
on_button_title_prev_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t * p_intf;
    input_area_t *  p_area;
    int             i_id;

    p_intf = GetIntf( GTK_WIDGET(button), "intf_window" );
    i_id = p_intf->p_input->stream.p_selected_area->i_id - 1;

    /* Disallow area 0 since it is used for video_ts.vob */
    if( i_id > 0 )
    {
        p_area = p_intf->p_input->stream.pp_areas[i_id];
        p_intf->p_input->pf_set_area( p_intf->p_input, (input_area_t*)p_area );

        input_SetStatus( p_intf->p_input, INPUT_STATUS_PLAY );

        p_intf->p_sys->b_title_update = 1;
    }
}


void
on_button_title_next_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t * p_intf;
    input_area_t *  p_area;
    int             i_id;

    p_intf = GetIntf( GTK_WIDGET(button), "intf_window" );
    i_id = p_intf->p_input->stream.p_selected_area->i_id + 1;

    if( i_id < p_intf->p_input->stream.i_area_nb )
    {
        p_area = p_intf->p_input->stream.pp_areas[i_id];   
        p_intf->p_input->pf_set_area( p_intf->p_input, (input_area_t*)p_area );

        input_SetStatus( p_intf->p_input, INPUT_STATUS_PLAY );

        p_intf->p_sys->b_title_update = 1;
    }
}


void
on_button_chapter_prev_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t * p_intf;
    input_area_t *  p_area;

    p_intf = GetIntf( GTK_WIDGET(button), "intf_window" );
    p_area = p_intf->p_input->stream.p_selected_area;

    if( p_area->i_part > 0 )
    {
        p_area->i_part--;
        p_intf->p_input->pf_set_area( p_intf->p_input, (input_area_t*)p_area );

        input_SetStatus( p_intf->p_input, INPUT_STATUS_PLAY );

        p_intf->p_sys->b_chapter_update = 1;
    }
}


void
on_button_chapter_next_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t * p_intf;
    input_area_t *  p_area;

    p_intf = GetIntf( GTK_WIDGET(button), "intf_window" );
    p_area = p_intf->p_input->stream.p_selected_area;

    if( p_area->i_part < p_area->i_part_nb )
    {
        p_area->i_part++;
        p_intf->p_input->pf_set_area( p_intf->p_input, (input_area_t*)p_area );

        input_SetStatus( p_intf->p_input, INPUT_STATUS_PLAY );

        p_intf->p_sys->b_chapter_update = 1;
    }
}


/*****************************************************************************
 * Menubar callbacks
 *****************************************************************************
 * The following callbacks are related to the menubar of the main
 * interface window.
 *****************************************************************************/
void
on_menubar_open_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_window" );

    /* If we have never used the file selector, open it */
    if( !GTK_IS_WIDGET( p_intf->p_sys->p_fileopen ) )
    {
        p_intf->p_sys->p_fileopen = create_intf_fileopen();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_fileopen ),
                             "p_intf", p_intf );

            gtk_file_selection_set_filename ( GTK_FILE_SELECTION(
                p_intf->p_sys->p_fileopen ),
                main_GetPszVariable( INTF_PATH_VAR, INTF_PATH_DEFAULT ) );
    }

    gtk_widget_show( p_intf->p_sys->p_fileopen );
    gdk_window_raise( p_intf->p_sys->p_fileopen->window );
}


void
on_menubar_disc_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_window" );

    /* If we have never used the file selector, open it */
    if( !GTK_IS_WIDGET( p_intf->p_sys->p_disc ) )
    {
        p_intf->p_sys->p_disc = create_intf_disc();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_disc ),
                             "p_intf", p_intf );
    }

    gtk_widget_show( p_intf->p_sys->p_disc );
    gdk_window_raise( p_intf->p_sys->p_disc->window );
}


void
on_menubar_network_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_window" );

    /* If we have never used the file selector, open it */
    if( !GTK_IS_WIDGET( p_intf->p_sys->p_network ) )
    {
        p_intf->p_sys->p_network = create_intf_network();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_network ),
                             "p_intf", p_intf );
    }

    gtk_widget_show( p_intf->p_sys->p_network );
    gdk_window_raise( p_intf->p_sys->p_network->window );
}


void
on_menubar_exit_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_window" );

    vlc_mutex_lock( &p_intf->change_lock );
    p_intf->b_die = 1;
    vlc_mutex_unlock( &p_intf->change_lock );
}


void
on_menubar_playlist_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t * p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_window" );
    playlist_t *    p_playlist ;
    GtkCList *      p_list;

    if( !GTK_IS_WIDGET( p_intf->p_sys->p_playlist ) )
    {
        p_intf->p_sys->p_playlist = create_intf_playlist();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_playlist ),
                             "p_intf", p_intf );
    }

    vlc_mutex_lock( &p_main->p_playlist->change_lock );
    if( p_main->p_playlist->i_size > 0 )
    {
        p_playlist = p_main->p_playlist;
        p_list = GTK_CLIST( lookup_widget( p_intf->p_sys->p_playlist,
                                           "playlist_clist" ) );
        GnomeRebuildCList( p_list, p_playlist );
    }
    vlc_mutex_unlock( &p_main->p_playlist->change_lock );

    gtk_widget_show( p_intf->p_sys->p_playlist );
    gdk_window_raise( p_intf->p_sys->p_playlist->window );
}


void
on_menubar_audio_toggle                (GtkCheckMenuItem     *menuitem,
                                        gpointer              user_data)
{
    intf_thread_t *         p_intf;
    es_descriptor_t *       p_es;

    p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_window" );

    if( !p_intf->p_sys->b_audio_update )
    {
        p_es = (es_descriptor_t*)user_data;

        input_ToggleES( p_intf->p_input, p_es, menuitem->active );

        p_intf->p_sys->b_audio_update = menuitem->active;
    }
}


void
on_menubar_subtitle_toggle             (GtkCheckMenuItem     *menuitem,
                                        gpointer              user_data)
{
    intf_thread_t *         p_intf;
    es_descriptor_t *       p_es;

    p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_window" );

    if( !p_intf->p_sys->b_spu_update )
    {
        p_es = (es_descriptor_t*)user_data;

        input_ToggleES( p_intf->p_input, p_es, menuitem->active );

        p_intf->p_sys->b_spu_update = menuitem->active;
    }
}


void
on_menubar_fullscreen_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    if( p_vout_bank->i_count )
    {
        vlc_mutex_lock( &p_vout_bank->pp_vout[0]->change_lock );
        if( p_vout_bank->pp_vout[0]->b_fullscreen ==
                !GTK_CHECK_MENU_ITEM( menuitem )->active )
            p_vout_bank->pp_vout[0]->i_changes |= VOUT_FULLSCREEN_CHANGE;
        vlc_mutex_unlock( &p_vout_bank->pp_vout[0]->change_lock );
    }
}



void
on_menubar_title_toggle                (GtkCheckMenuItem     *menuitem,
                                        gpointer              user_data)
{
    intf_thread_t * p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_window" );

    if( menuitem->active && !p_intf->p_sys->b_title_update )
    {
        gint i_title = (gint)user_data;
        p_intf->p_input->pf_set_area( p_intf->p_input,
            p_intf->p_input->stream.pp_areas[i_title] );

        input_SetStatus( p_intf->p_input, INPUT_STATUS_PLAY );

        p_intf->p_sys->b_title_update = 1;
    }
}


void
on_menubar_chapter_toggle              (GtkCheckMenuItem     *menuitem,
                                        gpointer              user_data)
{
    intf_thread_t * p_intf    = GetIntf( GTK_WIDGET(menuitem), "intf_window" );
    input_area_t *  p_area    = p_intf->p_input->stream.p_selected_area;
    gint            i_chapter = (gint)user_data;
    char            psz_chapter[5];

    if( menuitem->active && !p_intf->p_sys->b_chapter_update )
    {
        p_area->i_part = i_chapter;
        p_intf->p_input->pf_set_area( p_intf->p_input, (input_area_t*)p_area );

        snprintf( psz_chapter, 3, "%02d", p_area->i_part );
        gtk_label_set_text( p_intf->p_sys->p_label_chapter, psz_chapter );

        input_SetStatus( p_intf->p_input, INPUT_STATUS_PLAY );

        p_intf->p_sys->b_chapter_update = 1;
    }

}


void
on_menubar_angle_toggle                (GtkCheckMenuItem     *menuitem,
                                        gpointer             user_data)
{
    intf_thread_t * p_intf    = GetIntf( GTK_WIDGET(menuitem), "intf_window" );
    input_area_t *  p_area    = p_intf->p_input->stream.p_selected_area;
    gint            i_angle   = (gint)user_data;

    if( menuitem->active && !p_intf->p_sys->b_angle_update )
    {
        p_area->i_angle = i_angle;
        p_intf->p_input->pf_set_area( p_intf->p_input, (input_area_t*)p_area );

        p_intf->p_sys->b_angle_update = 1;
    }
}


void
on_menubar_modules_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_window" );

    if( !GTK_IS_WIDGET( p_intf->p_sys->p_modules ) )
    {
        p_intf->p_sys->p_modules = create_intf_modules();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_modules ),
                             "p_intf", p_intf );
    }
    gtk_widget_show( p_intf->p_sys->p_modules );
    gdk_window_raise( p_intf->p_sys->p_modules->window );
}


void
on_menubar_preferences_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_window" );

    /* If we have never used the file selector, open it */
    if( !GTK_IS_WIDGET( p_intf->p_sys->p_preferences ) )
    {
        p_intf->p_sys->p_preferences = create_intf_preferences();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_preferences ),
                             "p_intf", p_intf );
#if 1
#define ASSIGN_PSZ_ENTRY( var, default, name )                               \
    gtk_entry_set_text( GTK_ENTRY( gtk_object_get_data( GTK_OBJECT(          \
        p_intf->p_sys->p_preferences ), name ) ),                            \
                        main_GetPszVariable( var, default ) )

#define ASSIGN_INT_VALUE( var, default, name )                                        \
    gtk_spin_button_set_value( GTK_SPIN_BUTTON( gtk_object_get_data(         \
        GTK_OBJECT( p_intf->p_sys->p_preferences ), name ) ),                \
                               main_GetIntVariable( var, default ) )

#define ASSIGN_INT_TOGGLE( var, default, name )                                       \
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( gtk_object_get_data(    \
        GTK_OBJECT( p_intf->p_sys->p_preferences ), name ) ),                \
                               main_GetIntVariable( var, default ) )

        /* Default path */
        ASSIGN_PSZ_ENTRY( INTF_PATH_VAR, INTF_PATH_DEFAULT,
                          "preferences_file_entry" );
    
        /* Default DVD */
        ASSIGN_PSZ_ENTRY( INPUT_DVD_DEVICE_VAR,INPUT_DVD_DEVICE_DEFAULT,
                          "preferences_disc_dvd_entry" );
    
        /* Default VCD */
        ASSIGN_PSZ_ENTRY( INPUT_VCD_DEVICE_VAR, INPUT_VCD_DEVICE_DEFAULT,
                          "preferences_disc_vcd_entry" );
    
        /* Default server */
        ASSIGN_PSZ_ENTRY( INPUT_SERVER_VAR, INPUT_SERVER_DEFAULT,
                          "preferences_network_server_entry" );
    
        /* Default port */
        ASSIGN_INT_VALUE( INPUT_PORT_VAR, INPUT_PORT_DEFAULT,
                          "preferences_network_port_spinbutton" );
    
        /* Broadcast address */
        ASSIGN_PSZ_ENTRY( INPUT_BCAST_ADRR_VAR, INPUT_BCAST_ADDR_DEFAULT,
                          "preferences_network_broadcast_entry" );
    
        /* Broadcast stream by default ? */
        ASSIGN_INT_TOGGLE( INPUT_BROADCAST_VAR, INPUT_BROADCAST_DEFAULT,
                           "preferences_network_broadcast_checkbutton" );
    
        /* XXX Protocol */
    
        /* Default interface */
        ASSIGN_PSZ_ENTRY( INTF_METHOD_VAR, INTF_METHOD_DEFAULT,
                          "preferences_interface_entry" );
    
        /* Default video output */
        ASSIGN_PSZ_ENTRY( VOUT_METHOD_VAR, VOUT_METHOD_DEFAULT,
                          "preferences_video_output_entry" );
    
        /* Default output width */
        ASSIGN_INT_VALUE( VOUT_WIDTH_VAR, VOUT_WIDTH_DEFAULT,
                          "preferences_video_width_spinbutton" );
    
        /* Default output height */
        ASSIGN_INT_VALUE( VOUT_HEIGHT_VAR, VOUT_HEIGHT_DEFAULT,
                          "preferences_video_height_spinbutton" );
    
        /* XXX Default screen depth */
    
        /* XXX Default fullscreen depth */
    
        /* XXX Default gamma */
        
        /* Fullscreen on play */
        ASSIGN_INT_TOGGLE( VOUT_FULLSCREEN_VAR, VOUT_FULLSCREEN_DEFAULT,
                           "preferences_video_fullscreen_checkbutton" );
    
        /* Grayscale display */
        ASSIGN_INT_TOGGLE( VOUT_GRAYSCALE_VAR, VOUT_GRAYSCALE_DEFAULT,
                           "preferences_video_grayscale_checkbutton" );
    
        /* Default audio output */
        ASSIGN_PSZ_ENTRY( AOUT_METHOD_VAR, AOUT_METHOD_DEFAULT,
                          "preferences_audio_output_entry" );
    
        /* Default audio device */
        ASSIGN_PSZ_ENTRY( AOUT_DSP_VAR, AOUT_DSP_DEFAULT,
                          "preferences_audio_device_entry" );
    
        /* XXX Default frequency */
    
        /* XXX Default quality */
    
        /* XXX Default number of channels */
    
        /* Use spdif output ? */
        ASSIGN_INT_TOGGLE( AOUT_SPDIF_VAR, AOUT_SPDIF_DEFAULT,
                           "preferences_audio_spdif_checkbutton" );
    
        /* Launch playlist on startup */
        ASSIGN_INT_TOGGLE( PLAYLIST_STARTUP_VAR, PLAYLIST_STARTUP_DEFAULT,
                        "preferences_playlist_launch_on_startup_checkbutton" );
    
        /* Enqueue drag'n dropped item as default */
        ASSIGN_INT_TOGGLE( PLAYLIST_ENQUEUE_VAR, PLAYLIST_ENQUEUE_DEFAULT,
                           "preferences_playlist_enqueue_as_default_checkbutton" );
    
        /* Loop on playlist end */
        ASSIGN_INT_TOGGLE( PLAYLIST_LOOP_VAR, PLAYLIST_LOOP_DEFAULT,
                           "preferences_playlist_loop_checkbutton" );
    
        /* Verbosity of warning messages */
        ASSIGN_INT_VALUE( INTF_WARNING_VAR, INTF_WARNING_DEFAULT,
                          "preferences_misc_messages_spinbutton" );
#undef ASSIGN_PSZ_ENTRY
#undef ASSIGN_INT_VALUE
#undef ASSIGN_INT_TOGGLE
#endif
    }

    gtk_widget_show( p_intf->p_sys->p_preferences );
    gdk_window_raise( p_intf->p_sys->p_preferences->window );
}


void
on_menubar_about_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_window" );

    if( !GTK_IS_WIDGET( p_intf->p_sys->p_about ) )
    {
        p_intf->p_sys->p_about = create_intf_about();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_about ),
                             "p_intf", p_intf );
    }
    gtk_widget_show( p_intf->p_sys->p_about );
    gdk_window_raise( p_intf->p_sys->p_about->window );
}


/*****************************************************************************
 * Toolbar callbacks
 *****************************************************************************
 * The following callbacks are related to the toolbar of the main
 * interface window.
 *****************************************************************************/
void
on_toolbar_open_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(button), "intf_window" );

    /* If we have never used the file selector, open it */
    if( !GTK_IS_WIDGET( p_intf->p_sys->p_fileopen ) )
    {
        p_intf->p_sys->p_fileopen = create_intf_fileopen();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_fileopen ),
                             "p_intf", p_intf );

        if( main_GetPszVariable( INTF_PATH_VAR, INTF_PATH_DEFAULT ) != NULL )
        {
            gtk_file_selection_set_filename( GTK_FILE_SELECTION(
                p_intf->p_sys->p_fileopen ),
                main_GetPszVariable( INTF_PATH_VAR, INTF_PATH_DEFAULT ) );
        }
    }

    gtk_widget_show( p_intf->p_sys->p_fileopen );
    gdk_window_raise( p_intf->p_sys->p_fileopen->window );
}


void
on_toolbar_disc_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(button), "intf_window" );

    /* If we have never used the file selector, open it */
    if( !GTK_IS_WIDGET( p_intf->p_sys->p_disc ) )
    {
        p_intf->p_sys->p_disc = create_intf_disc();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_disc ),
                             "p_intf", p_intf );
    }

    gtk_widget_show( p_intf->p_sys->p_disc );
    gdk_window_raise( p_intf->p_sys->p_disc->window );
}


void
on_toolbar_network_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(button), "intf_window" );

    /* If we have never used the file selector, open it */
    if( !GTK_IS_WIDGET( p_intf->p_sys->p_network ) )
    {
        p_intf->p_sys->p_network = create_intf_network();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_network ),
                             "p_intf", p_intf );
    }

    gtk_widget_show( p_intf->p_sys->p_network );
    gdk_window_raise( p_intf->p_sys->p_network->window );
}


void
on_toolbar_back_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_toolbar_stop_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(button), "intf_window" );

    if( p_intf->p_input != NULL )
    {
        p_intf->p_input->b_eof = 1;
        p_main->p_playlist->i_index--;
    }
    p_main->p_playlist->b_stopped = 1;
}


void
on_toolbar_play_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(button), "intf_window" );

    if( p_intf->p_input != NULL )
    {
        input_SetStatus( p_intf->p_input, INPUT_STATUS_PLAY );
    }
    else if( p_main->p_playlist->b_stopped )
    {
        if( p_main->p_playlist->i_size )
        {
            intf_PlaylistJumpto( p_main->p_playlist,
                                 p_main->p_playlist->i_index );
            p_main->p_playlist->b_stopped = 0;
        }
        else
        {
            on_toolbar_open_clicked( button, user_data );
        }
    }

}


void
on_toolbar_pause_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(button), "intf_window" );

    if( p_intf->p_input != NULL )
    {
        input_SetStatus( p_intf->p_input, INPUT_STATUS_PAUSE );
    }
}


void
on_toolbar_slow_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(button), "intf_window" );

    if( p_intf->p_input != NULL )
    {
        input_SetStatus( p_intf->p_input, INPUT_STATUS_SLOWER );
    }
}


void
on_toolbar_fast_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(button), "intf_window" );

    if( p_intf->p_input != NULL )
    {
        input_SetStatus( p_intf->p_input, INPUT_STATUS_FASTER );
    }
}


void
on_toolbar_playlist_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(button), "intf_window" );

    if( !GTK_IS_WIDGET( p_intf->p_sys->p_playlist ) )
    {
        p_intf->p_sys->p_playlist = create_intf_playlist();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_playlist ),
                             "p_intf", p_intf );
    }

    if( GTK_WIDGET_VISIBLE( p_intf->p_sys->p_playlist ) )
    {
        gtk_widget_hide( p_intf->p_sys->p_playlist );
    } 
    else 
    {        
        GtkCList * p_clist;

        p_clist = GTK_CLIST( gtk_object_get_data(
            GTK_OBJECT( p_intf->p_sys->p_playlist ), "playlist_clist" ) );
        GnomeRebuildCList( p_clist , p_main->p_playlist );
        gtk_widget_show( p_intf->p_sys->p_playlist );
        gdk_window_raise( p_intf->p_sys->p_playlist->window );
    }
}


void
on_toolbar_prev_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(button), "intf_window" );

    if( p_intf->p_input != NULL )
    {
        /* FIXME: temporary hack */
        intf_PlaylistPrev( p_main->p_playlist );
        intf_PlaylistPrev( p_main->p_playlist );
        p_intf->p_input->b_eof = 1;
    }

    p_intf->p_sys->b_mode_changed = 1;
}


void
on_toolbar_next_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(button), "intf_window" );

    if( p_intf->p_input != NULL )
    {
        /* FIXME: temporary hack */
        p_intf->p_input->b_eof = 1;
    }

    p_intf->p_sys->b_mode_changed = 1;
}


/*****************************************************************************
 * Popup callbacks
 *****************************************************************************
 * The following callbacks are related to the popup menu. The popup
 * menu is activated when right-clicking on the video output window.
 *****************************************************************************/
void
on_popup_play_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_popup" );

    if( p_intf->p_input != NULL )
    {
        input_SetStatus( p_intf->p_input, INPUT_STATUS_PLAY );
        p_main->p_playlist->b_stopped = 0;
    }
}


void
on_popup_back_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_popup_pause_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_popup" );

    if( p_intf->p_input != NULL )
    {
        input_SetStatus( p_intf->p_input, INPUT_STATUS_PAUSE );
    }
}


void
on_popup_slow_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_popup" );

    if( p_intf->p_input != NULL )
    {
        input_SetStatus( p_intf->p_input, INPUT_STATUS_SLOWER );
    }
}


void
on_popup_fast_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_popup" );

    if( p_intf->p_input != NULL )
    {
        input_SetStatus( p_intf->p_input, INPUT_STATUS_FASTER );
    }
}


void
on_popup_fullscreen_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    if( p_vout_bank->i_count )
    {
        vlc_mutex_lock( &p_vout_bank->pp_vout[0]->change_lock );
        if( p_vout_bank->pp_vout[0]->b_fullscreen ==
                !GTK_CHECK_MENU_ITEM( menuitem )->active )
            p_vout_bank->pp_vout[0]->i_changes |= VOUT_FULLSCREEN_CHANGE;
        vlc_mutex_unlock( &p_vout_bank->pp_vout[0]->change_lock );
    }
}


void
on_popup_audio_toggle                  (GtkCheckMenuItem     *menuitem,
                                        gpointer              user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_popup" );
    es_descriptor_t *       p_es;

    p_es = (es_descriptor_t*)user_data;

    if( !p_intf->p_sys->b_audio_update )
    {
        input_ToggleES( p_intf->p_input, p_es, menuitem->active );

        p_intf->p_sys->b_audio_update = menuitem->active;
    }
}


void
on_popup_subtitle_toggle            (GtkCheckMenuItem     *menuitem,
                                     gpointer              user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_popup" );
    es_descriptor_t *       p_es;

    p_es = (es_descriptor_t*)user_data;

    if( !p_intf->p_sys->b_spu_update )
    {
        input_ToggleES( p_intf->p_input, p_es, menuitem->active );

        p_intf->p_sys->b_spu_update = menuitem->active;
    }
}


void
on_popup_navigation_toggle             (GtkCheckMenuItem     *menuitem,
                                        gpointer             user_data)
{
    intf_thread_t * p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_popup" );

    if( menuitem->active &&
        !p_intf->p_sys->b_title_update &&
        !p_intf->p_sys->b_chapter_update )
    {
        input_area_t   *p_area = p_intf->p_input->stream.p_selected_area;

        gint i_title   = DATA2TITLE( user_data );
        gint i_chapter = DATA2CHAPTER( user_data );

        if( p_area != p_intf->p_input->stream.pp_areas[i_title] )
        {
            p_area = p_intf->p_input->stream.pp_areas[i_title];
            p_intf->p_sys->b_title_update = 1;
        }

        p_area->i_part = i_chapter;
        p_intf->p_sys->b_chapter_update = 1;

        p_intf->p_input->pf_set_area( p_intf->p_input, (input_area_t*)p_area );

        input_SetStatus( p_intf->p_input, INPUT_STATUS_PLAY );
    }
}


void
on_popup_angle_toggle                  (GtkCheckMenuItem     *menuitem,
                                        gpointer             user_data)
{
    intf_thread_t * p_intf    = GetIntf( GTK_WIDGET(menuitem), "intf_popup" );
    input_area_t *  p_area    = p_intf->p_input->stream.p_selected_area;
    gint            i_angle   = (gint)user_data;

    if( menuitem->active && !p_intf->p_sys->b_angle_update )
    {
        p_area->i_angle = i_angle;
        p_intf->p_input->pf_set_area( p_intf->p_input, (input_area_t*)p_area );

        p_intf->p_sys->b_angle_update = 1;
    }
}


void
on_popup_open_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_popup" );

    /* If we have never used the file selector, open it */
    if( !GTK_IS_WIDGET( p_intf->p_sys->p_fileopen ) )
    {
        p_intf->p_sys->p_fileopen = create_intf_fileopen();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_fileopen ),
                             "p_intf", p_intf );

        if( main_GetPszVariable( INTF_PATH_VAR, INTF_PATH_DEFAULT ) != NULL )
        {
            gtk_file_selection_set_filename( GTK_FILE_SELECTION(
                p_intf->p_sys->p_fileopen ),
                main_GetPszVariable( INTF_PATH_VAR, INTF_PATH_DEFAULT ) );
        }
    }

    gtk_widget_show( p_intf->p_sys->p_fileopen );
    gdk_window_raise( p_intf->p_sys->p_fileopen->window );
}


void
on_popup_disc_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_popup" );

    gtk_widget_show( p_intf->p_sys->p_disc );
    gdk_window_raise( p_intf->p_sys->p_disc->window );
}


void
on_popup_network_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_popup" );

    gtk_widget_show( p_intf->p_sys->p_network );
    gdk_window_raise( p_intf->p_sys->p_network->window );
}


void
on_popup_about_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_popup" );

    if( !GTK_IS_WIDGET( p_intf->p_sys->p_about ) )
    {
        p_intf->p_sys->p_about = create_intf_about();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_about ),
                             "p_intf", p_intf );
    }
    gtk_widget_show( p_intf->p_sys->p_about );
    gdk_window_raise( p_intf->p_sys->p_about->window );
}


void
on_popup_exit_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_popup" );

    vlc_mutex_lock( &p_intf->change_lock );
    p_intf->b_die = 1;
    vlc_mutex_unlock( &p_intf->change_lock );
}


void
on_popup_stop_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_popup" );

    if( p_intf->p_input != NULL )
    {
        p_intf->p_input->b_die = 1;
    }
}


void
on_popup_playlist_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_window" );

    if( !GTK_IS_WIDGET( p_intf->p_sys->p_playlist ) )
    {
        p_intf->p_sys->p_playlist = create_intf_playlist();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_playlist ),
                             "p_intf", p_intf );
    }
    gtk_widget_show( p_intf->p_sys->p_playlist );
    gdk_window_raise( p_intf->p_sys->p_playlist->window );
}


void
on_popup_jump_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_popup" );

    if( !GTK_IS_WIDGET( p_intf->p_sys->p_jump ) )
    {
        p_intf->p_sys->p_jump = create_intf_jump();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_jump ),
                             "p_intf", p_intf );
    }

    gtk_widget_show( p_intf->p_sys->p_jump );
    gdk_window_raise( p_intf->p_sys->p_jump->window );
}


void
on_popup_preferences_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(menuitem), "intf_popup" );

    gtk_widget_show( p_intf->p_sys->p_preferences );
    gdk_window_raise( p_intf->p_sys->p_preferences->window );

}


/*****************************************************************************
 * Fileopen callbacks
 *****************************************************************************
 * The following callbacks are related to the file requester.
 *****************************************************************************/
void
on_intf_fileopen_destroy               (GtkObject       *object,
                                        gpointer         user_data)
{
    gtk_widget_hide( GTK_WIDGET( object ) );
}


void
on_fileopen_ok_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t * p_intf = GetIntf( GTK_WIDGET(button), "intf_fileopen" );
    GtkCList *      p_playlist_clist;
    GtkWidget *     p_filesel;
    gchar *         psz_filename;
    int             i_end = p_main->p_playlist->i_size;

    p_filesel = gtk_widget_get_toplevel( GTK_WIDGET(button) );
    gtk_widget_hide( p_filesel );
    psz_filename =
        gtk_file_selection_get_filename( GTK_FILE_SELECTION( p_filesel ) );

    intf_PlaylistAdd( p_main->p_playlist, PLAYLIST_END, (char*)psz_filename );

    /* catch the GTK CList */
    p_playlist_clist = GTK_CLIST( gtk_object_get_data(
        GTK_OBJECT( p_intf->p_sys->p_playlist ), "playlist_clist" ) );
    /* update the display */
    GnomeRebuildCList( p_playlist_clist, p_main->p_playlist );

    /* Select added item and switch to file interface */
    if( p_intf->p_input != NULL )
    {
        p_intf->p_input->b_eof = 1;
    }

    intf_PlaylistJumpto( p_main->p_playlist, i_end - 1 );
    p_main->p_playlist->b_stopped = 0;
}


void
on_fileopen_cancel_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
    gtk_widget_hide( gtk_widget_get_toplevel( GTK_WIDGET (button) ) );
}


/*****************************************************************************
 * Module manager callbacks
 *****************************************************************************
 * The following callbacks are related to the module manager.
 *****************************************************************************/
void
on_intf_modules_destroy                (GtkObject       *object,
                                        gpointer         user_data)
{

}


void
on_modules_ok_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(button), "intf_modules" );

    gtk_widget_hide( p_intf->p_sys->p_modules );
}


void
on_modules_apply_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_modules_cancel_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
    gtk_widget_hide( gtk_widget_get_toplevel( GTK_WIDGET (button) ) );
}


/*****************************************************************************
 * Open disc callbacks
 *****************************************************************************
 * The following callbacks are related to the disc manager.
 *****************************************************************************/
void
on_disc_dvd_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    if( togglebutton->active )
    {
        gtk_entry_set_text( GTK_ENTRY( lookup_widget(
            GTK_WIDGET(togglebutton), "disc_name" ) ), "/dev/dvd" );
    }
}


void
on_disc_vcd_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    if( togglebutton->active )
    {
        gtk_entry_set_text( GTK_ENTRY( lookup_widget(
            GTK_WIDGET(togglebutton), "disc_name" ) ), "/dev/cdrom" );
    }
}


void
on_disc_ok_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t * p_intf = GetIntf( GTK_WIDGET(button), "intf_disc" );
    GtkCList *      p_playlist_clist;
    char *          psz_device, *psz_source, *psz_method;
    int             i_end = p_main->p_playlist->i_size;

    gtk_widget_hide( p_intf->p_sys->p_disc );
    psz_device = gtk_entry_get_text( GTK_ENTRY( lookup_widget(
                                         GTK_WIDGET(button), "disc_name" ) ) );

    /* "dvd:foo" has size 5 + strlen(foo) */
    psz_source = malloc( 3 /* "dvd" */ + 1 /* ":" */
                           + strlen( psz_device ) + 1 /* "\0" */ );
    if( psz_source == NULL )
    {
        return;
    }

    /* Check which method was activated */
    if( GTK_TOGGLE_BUTTON( lookup_widget( GTK_WIDGET(button),
                                          "disc_dvd" ) )->active )
    {
        psz_method = "dvd";
        p_intf->p_sys->i_intf_mode = DVD_MODE;
    }
    else if( GTK_TOGGLE_BUTTON( lookup_widget( GTK_WIDGET(button),
                                               "disc_vcd" ) )->active )
    {
        psz_method = "vcd";
    }
    else
    {
        intf_ErrMsg( "intf error: unknown disc type toggle button position" );
        free( psz_source );
        return;
    }
    
    /* Select title and chapter */
    main_PutIntVariable( INPUT_TITLE_VAR, gtk_spin_button_get_value_as_int(
                              GTK_SPIN_BUTTON( lookup_widget(
                                  GTK_WIDGET(button), "disc_title" ) ) ) );

    main_PutIntVariable( INPUT_CHAPTER_VAR, gtk_spin_button_get_value_as_int(
                              GTK_SPIN_BUTTON( lookup_widget(
                                  GTK_WIDGET(button), "disc_chapter" ) ) ) );

    /* Build source name and add it to playlist */
    sprintf( psz_source, "%s:%s", psz_method, psz_device );
    intf_PlaylistAdd( p_main->p_playlist, PLAYLIST_END, psz_source );
    free( psz_source );

    /* catch the GTK CList */
    p_playlist_clist = GTK_CLIST( gtk_object_get_data(
        GTK_OBJECT( p_intf->p_sys->p_playlist ), "playlist_clist" ) );
    /* update the display */
    GnomeRebuildCList( p_playlist_clist, p_main->p_playlist );

    /* Select added item and switch to DVD interface */
    if( p_intf->p_input != NULL )
    {
        p_intf->p_input->b_eof = 1;
    }

    intf_PlaylistJumpto( p_main->p_playlist, i_end - 1 );
    p_main->p_playlist->b_stopped = 0;
}


void
on_disc_cancel_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
    gtk_widget_hide( gtk_widget_get_toplevel( GTK_WIDGET (button) ) );
}


/*****************************************************************************
 * Network stream callbacks
 *****************************************************************************
 * The following callbacks are related to the network stream manager.
 *****************************************************************************/
void
on_network_ok_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
    intf_thread_t * p_intf = GetIntf( GTK_WIDGET(button), "intf_network" );
    GtkCList *      p_playlist_clist;
    char *          psz_source, *psz_server, *psz_protocol;
    unsigned int    i_port;
    boolean_t       b_broadcast;
    int             i_end = p_main->p_playlist->i_size;

    gtk_widget_hide( p_intf->p_sys->p_network );
    psz_server = gtk_entry_get_text( GTK_ENTRY( lookup_widget(
                                 GTK_WIDGET(button), "network_server" ) ) );

    /* Check which protocol was activated */
    if( GTK_TOGGLE_BUTTON( lookup_widget( GTK_WIDGET(button),
                                          "network_ts" ) )->active )
    {
        psz_protocol = "ts";
    }
    else if( GTK_TOGGLE_BUTTON( lookup_widget( GTK_WIDGET(button),
                                               "network_rtp" ) )->active )
    {
        psz_protocol = "rtp";
    }
    else
    {
        intf_ErrMsg( "intf error: unknown protocol toggle button position" );
        return;
    }

    /* Get the port number and make sure it will not overflow 5 characters */
    i_port = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(
                 lookup_widget( GTK_WIDGET(button), "network_port" ) ) );
    if( i_port > 65535 )
    {
        intf_ErrMsg( "intf error: invalid port %i", i_port );
    }

    /* do we have a broadcast address */
    b_broadcast = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(
        lookup_widget( GTK_WIDGET(button), "broadcast_check" ) ) );
    if( b_broadcast )
    {
        char *  psz_broadcast;
        psz_broadcast = gtk_entry_get_text( GTK_ENTRY( lookup_widget(
                                 GTK_WIDGET(button), "network_broadcast" ) ) );
        /* Allocate room for "protocol://server:port" */
        psz_source = malloc( strlen( psz_protocol ) + 3 /* "://" */
                               + strlen( psz_server ) + 1 /* ":" */
                               + 5 /* 0-65535 */
                               + strlen( psz_broadcast ) + 2 /* "::" */ 
                               + 1 /* "\0" */ );
        if( psz_source == NULL )
        {
            return;
        }

        /* Build source name and add it to playlist */
        sprintf( psz_source, "%s://%s:%i::%s", psz_protocol,
                                               psz_server,
                                               i_port,
                                               psz_broadcast );
    }
    else
    {
        /* Allocate room for "protocol://server:port" */
        psz_source = malloc( strlen( psz_protocol ) + 3 /* "://" */
                               + strlen( psz_server ) + 1 /* ":" */
                               + 5 /* 0-65535 */ + 1 /* "\0" */ );
        if( psz_source == NULL )
        {
            return;
        }
       
        /* Build source name and add it to playlist */
        sprintf( psz_source, "%s://%s:%i", psz_protocol, psz_server, i_port );
    }

    intf_PlaylistAdd( p_main->p_playlist, PLAYLIST_END, psz_source );
    free( psz_source );

    /* catch the GTK CList */
    p_playlist_clist = GTK_CLIST( gtk_object_get_data(
        GTK_OBJECT( p_intf->p_sys->p_playlist ), "playlist_clist" ) );
    /* update the display */
    GnomeRebuildCList( p_playlist_clist, p_main->p_playlist );

    /* Select added item and switch to network interface */
    if( p_intf->p_input != NULL )
    {
        p_intf->p_input->b_eof = 1;
    }

    intf_PlaylistJumpto( p_main->p_playlist, i_end - 1 );
    p_main->p_playlist->b_stopped = 0;
}


void
on_network_cancel_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
    gtk_widget_hide( gtk_widget_get_toplevel( GTK_WIDGET (button) ) );
}


void
on_broadcast_check_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    GtkWidget *     p_network;

    p_network = gtk_widget_get_toplevel( GTK_WIDGET (togglebutton) );
    gtk_widget_set_sensitive( gtk_object_get_data( GTK_OBJECT( p_network ),
            "network_broadcast_combo" ),
            gtk_toggle_button_get_active( togglebutton ) );
    gtk_widget_set_sensitive( gtk_object_get_data( GTK_OBJECT( p_network ),
            "network_broadcast" ),
            gtk_toggle_button_get_active( togglebutton ) );
}


/****************************************************************************
 * Jump window
 ****************************************************************************
 * Those callback are received from the jump window that allow to select
 * an arbitrary position of the stream.
 ****************************************************************************/
void
on_jump_ok_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
    on_jump_apply_clicked( button, user_data );
    gtk_widget_hide( gtk_widget_get_toplevel( GTK_WIDGET (button) ) );
}


void
on_jump_apply_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
#if 1
    intf_thread_t * p_intf;
    off_t           i_seek;
    off_t           i_size;
    int             i_hours;
    int             i_minutes;
    int             i_seconds;

    p_intf = GetIntf( GTK_WIDGET( button ), "intf_jump" );

#define GET_VALUE( name )                                                   \
    gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( gtk_object_get_data( \
        GTK_OBJECT( p_intf->p_sys->p_jump ), name ) ) )

    i_hours   = GET_VALUE( "jump_hour_spinbutton" );
    i_minutes = GET_VALUE( "jump_minute_spinbutton" );
    i_seconds = GET_VALUE( "jump_second_spinbutton" );

#undef GET_VALUE

    i_seconds += 60 *i_minutes + 3600* i_hours;

    vlc_mutex_lock( &p_intf->p_input->stream.stream_lock );
    i_seek = i_seconds * 50 * p_intf->p_input->stream.i_mux_rate;
    i_size = p_intf->p_input->stream.p_selected_area->i_size;
    vlc_mutex_unlock( &p_intf->p_input->stream.stream_lock );

    if( i_seek < i_size )
    {
        input_Seek( p_intf->p_input, i_seek );
    }
    p_main->p_playlist->b_stopped = 0;
#endif
}


void
on_jump_cancel_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
    gtk_widget_hide( gtk_widget_get_toplevel( GTK_WIDGET (button) ) );
}


/****************************************************************************
 * Preferences window callbacks
 ****************************************************************************/
void
on_preferences_ok_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
    on_preferences_apply_clicked( button, user_data );
    gtk_widget_hide( gtk_widget_get_toplevel( GTK_WIDGET (button) ) );
}


void
on_preferences_apply_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
#if 1
    GtkWidget *     p_preferences;

    /* get preferences window */
    p_preferences = gtk_widget_get_toplevel( GTK_WIDGET( button ) );

#define ASSIGN_PSZ_ENTRY( var, name )                                        \
    main_PutPszVariable( var, gtk_entry_get_text(                            \
    GTK_ENTRY( gtk_object_get_data( GTK_OBJECT( p_preferences ), name ) ) ) )

#define ASSIGN_INT_VALUE( var, name )                                        \
    main_PutIntVariable( var, gtk_spin_button_get_value_as_int(              \
        GTK_SPIN_BUTTON( gtk_object_get_data( GTK_OBJECT( p_preferences ),   \
        name ) ) ) )

#define ASSIGN_INT_TOGGLE( var, name )                                       \
    main_PutIntVariable( var, gtk_toggle_button_get_active(                  \
        GTK_TOGGLE_BUTTON( gtk_object_get_data( GTK_OBJECT( p_preferences ), \
        name ) ) ) )

    /* Default path */
    ASSIGN_PSZ_ENTRY( INTF_PATH_VAR, "preferences_file_entry" );

    /* Default DVD */
    ASSIGN_PSZ_ENTRY( INPUT_DVD_DEVICE_VAR, "preferences_disc_dvd_entry" );

    /* Default VCD */
    ASSIGN_PSZ_ENTRY( INPUT_VCD_DEVICE_VAR, "preferences_disc_vcd_entry" );

    /* Default server */
    ASSIGN_PSZ_ENTRY( INPUT_SERVER_VAR, "preferences_network_server_entry" );

    /* Default port */
    ASSIGN_INT_VALUE( INPUT_PORT_VAR, "preferences_network_port_spinbutton" );

    /* Broadcast address */
    ASSIGN_PSZ_ENTRY( INPUT_BCAST_ADRR_VAR,
                      "preferences_network_broadcast_entry" );

    /* Broadcast stream by default ? */
    ASSIGN_INT_TOGGLE( INPUT_BROADCAST_VAR,
                       "preferences_network_broadcast_checkbutton" );

    /* XXX Protocol */

    /* Default interface */
    ASSIGN_PSZ_ENTRY( INTF_METHOD_VAR, "preferences_interface_entry" );

    /* Default video output */
    ASSIGN_PSZ_ENTRY( VOUT_METHOD_VAR, "preferences_video_output_entry" );

    /* Default output width */
    ASSIGN_INT_VALUE( VOUT_WIDTH_VAR, "preferences_video_width_spinbutton" );

    /* Default output height */
    ASSIGN_INT_VALUE( VOUT_HEIGHT_VAR, "preferences_video_height_spinbutton" );

    /* XXX Default screen depth */

    /* XXX Default fullscreen depth */

    /* XXX Default gamma */
    
    /* Fullscreen on play */
    ASSIGN_INT_TOGGLE( VOUT_FULLSCREEN_VAR,
                       "preferences_video_fullscreen_checkbutton" );

    /* Grayscale display */
    ASSIGN_INT_TOGGLE( VOUT_GRAYSCALE_VAR,
                       "preferences_video_grayscale_checkbutton" );

    /* Default audio output */
    ASSIGN_PSZ_ENTRY( AOUT_METHOD_VAR, "preferences_audio_output_entry" );

    /* Default audio device */
    ASSIGN_PSZ_ENTRY( AOUT_DSP_VAR, "preferences_audio_device_entry" );

    /* XXX Default frequency */

    /* XXX Default quality */

    /* XXX Default number of channels */

    /* Use spdif output ? */
    ASSIGN_INT_TOGGLE( AOUT_SPDIF_VAR, "preferences_audio_spdif_checkbutton" );

    /* Launch playlist on startup */
    ASSIGN_INT_TOGGLE( PLAYLIST_STARTUP_VAR,
                       "preferences_playlist_launch_on_startup_checkbutton" );

    /* Enqueue drag'n dropped item as default */
    ASSIGN_INT_TOGGLE( PLAYLIST_ENQUEUE_VAR,
                       "preferences_playlist_enqueue_as_default_checkbutton" );

    /* Loop on playlist end */
    ASSIGN_INT_TOGGLE( PLAYLIST_LOOP_VAR,
                       "preferences_playlist_loop_checkbutton" );

    /* Verbosity of warning messages */
    ASSIGN_INT_VALUE( INTF_WARNING_VAR,
                      "preferences_misc_messages_spinbutton" );
#undef ASSIGN_PSZ_ENTRY
#undef ASSIGN_INT_VALUE
#undef ASSIGN_INT_TOGGLE
#endif
}


void
on_preferences_cancel_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
    gtk_widget_hide( gtk_widget_get_toplevel( GTK_WIDGET (button) ) );
}
