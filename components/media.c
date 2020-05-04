#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpd/client.h>

#include "../util.h"

void
format_title(char *res, const char *title, size_t n)
{
    if (strlen(title) > n) {
        strncpy(res, title, n - 4);
        memset(res + n - 4, '.', 3);
        res[n] = '\0';
        return;
    }

    strcpy(res, title);
}

const char *
media_mpd_stat()
{
        struct mpd_connection *conn;
        struct mpd_status *status;
        struct mpd_song *song;
        char *mpc_title, title[16];
        char *ret;
        unsigned state;
        int elapsed, total;

        ret = (char *)malloc(128);
        if (!((conn = mpd_connection_new(NULL, 0, 0)) || 
                    mpd_connection_get_error(conn)))
                return NULL;

        mpd_command_list_begin(conn, true);
        mpd_send_status(conn);
        mpd_send_current_song(conn);
        mpd_command_list_end(conn);
        status = mpd_recv_status(conn);
        if (status) {
                state = mpd_status_get_state(status);
                if (state == MPD_STATE_PLAY) {
                        mpd_response_next(conn);
                        song = mpd_recv_song(conn);
                        mpc_title = (char *)mpd_song_get_tag(song,
                                MPD_TAG_TITLE, 0);
                        format_title(title, mpc_title, sizeof(title));
                        elapsed = mpd_status_get_elapsed_time(status);
                        total = mpd_status_get_total_time(status);
                        snprintf(ret, 128, "\uf01d %s (%.2d:%.2d/%.2d:%.2d)",
                                title, elapsed / 60, elapsed %60,
                                total / 60, total %60);
                        free(mpc_title);
                } else if (state == MPD_STATE_PAUSE) {
                        mpd_response_next(conn);
                        song = mpd_recv_song(conn);
                        mpc_title = (char *)mpd_song_get_tag(song,
                                MPD_TAG_TITLE, 0);
                        format_title(title, mpc_title, sizeof(title));
                        elapsed = mpd_status_get_elapsed_time(status);
                        total = mpd_status_get_total_time(status);
                        snprintf(ret, 128, "\uf28c %s (%.2d:%.2d/%.2d:%.2d)",
                                title, elapsed / 60, elapsed %60,
                                total / 60, total %60);
                        free(mpc_title);
                } else if (state == MPD_STATE_STOP) {
                        mpd_response_next(conn);
                        if ((song = mpd_recv_song(conn))) {
                            mpc_title = (char *)mpd_song_get_tag(song,
                                    MPD_TAG_TITLE, 0);
                            format_title(title, mpc_title, sizeof(title));
                            snprintf(ret, 128, "\uf28e %s ", title);
                            free(mpc_title);
                        } else {
                            snprintf(ret, 128, "\ue374 ");
                        }
                } else {
                        /* state == MPD_STATE_UNKNOWN */
                        snprintf(ret, 128, "\uf29c ");
                } 
        } else {
                return NULL;
        }
        mpd_response_finish(conn);
        mpd_connection_free(conn);

        return ret;
}
