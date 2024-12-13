#include <fcntl.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>

// Structure pour stocker les informations DRM
typedef struct {
    int fd;
    drmModeModeInfo mode;
    uint32_t fb;
    uint32_t conn_id;
    uint32_t crtc_id;
    void *map;
    size_t size;
} DRM;

int init_drm(DRM *drm, const char *device) {
    drm->fd = open(device, O_RDWR | O_CLOEXEC);
    if (drm->fd < 0) {
        perror("Cannot open DRM device");
        return -1;
    }

    drmModeRes *res = drmModeGetResources(drm->fd);
    if (!res) {
        fprintf(stderr, "drmModeGetResources failed\n");
        return -1;
    }
    drm->conn_id = res->connectors[0];
    drmModeConnector *conn = drmModeGetConnector(drm->fd, drm->conn_id);
    drmModeEncoder *enc = drmModeGetEncoder(drm->fd, conn->encoder_id);

    drm->crtc_id = enc->crtc_id;
    drm->mode = conn->modes[0];

    drmModeFreeResources(res);
    drmModeFreeConnector(conn);
    drmModeFreeEncoder(enc);

    drm->size = drm->mode.hdisplay * drm->mode.vdisplay * 4; // 4 bytes per pixel (RGBA)
    drm->map = mmap(NULL, drm->size, PROT_READ | PROT_WRITE, MAP_SHARED, drm->fd, 0);

    return 0;
}

void draw_letter(DRM *drm, char letter, int x, int y) {
    // Simple bitmap font for demonstration
    const int LETTER_WIDTH = 8;
    const int LETTER_HEIGHT = 8;
    uint8_t font[256][8] = {
        ['A'] = {0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00},
        // Add more letters here...
    };

    uint32_t *framebuffer = (uint32_t *)drm->map;
    for (int dy = 0; dy < LETTER_HEIGHT; dy++) {
        for (int dx = 0; dx < LETTER_WIDTH; dx++) {
            if (font[(int)letter][dy] & (1 << (7 - dx))) {
                int px = (y + dy) * drm->mode.hdisplay + (x + dx);
                framebuffer[px] = 0xFFFFFFFF; // White pixel
            }
        }
    }
}

int main() {
    printf("start main function\n");
    DRM drm;
    if (init_drm(&drm, "/dev/dri/card0") < 0) return -1;
    printf("init drm OK !\n");
    memset(drm.map, 0, drm.size); // Clear the screen
    printf("start printing\n");
    draw_letter(&drm, 'A', 100, 100);
    printf("letter A OK!\n");
    draw_letter(&drm, 'B', 120, 100);

    getchar(); // Wait for keypress

    munmap(drm.map, drm.size);
    close(drm.fd);
    return 0;
}

