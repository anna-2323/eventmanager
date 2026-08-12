#include "qrcode.h"

int generate_ticket_qr(const char* token, char* path, size_t path_size) {
    char payload[64];
    snprintf(payload, sizeof(payload), "ticket:%s", token);

    QRcode* qr = QRcode_encodeString8bit(
        payload,
        0,
        QR_ECLEVEL_M
    );
    if (!qr)
        return 0;

    snprintf(path, path_size, "tickets/qr/%s.svg", token);
    FILE* file = fopen(path, "w");
    if (!file) {
        QRcode_free(qr);
        return 0;
    }

    // QR кодът се генерира твърде малък, затова желаният размер се задава в
    // target_size, който става width и height на svg елемента.
    // С preserveAspectRatio QR кода се мащабира до target_size.
    const int target_size = 200;
    const int margin_modules = 4;
    int qr_size = qr->width + margin_modules * 2;

    fprintf(file,
        "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        "width=\"%d\" height=\"%d\" "
        "viewBox=\"0 0 %d %d\" "
        "preserveAspectRatio=\"xMidYMid meet\" "
        "shape-rendering=\"crispEdges\">\n",
        target_size, target_size, qr_size, qr_size);

    fprintf(file,
        "<rect width=\"100%%\" height=\"100%%\" fill=\"white\"/>\n");

    for (int y = 0; y < qr->width; y++) {
        for (int x = 0; x < qr->width; x++) {
            if (qr->data[y * qr->width + x] & 1) {
                fprintf(file,
                    "<rect x=\"%d\" y=\"%d\" width=\"1\" height=\"1\"/>\n",
                    x + margin_modules,
                    y + margin_modules);
            }
        }
    }

    fprintf(file, "</svg>\n");

    fclose(file);
    QRcode_free(qr);

    return 1;
}