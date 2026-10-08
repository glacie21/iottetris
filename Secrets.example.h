/*
 * ============================================================
 *  Secrets.example.h  –  Template kredensial
 * ============================================================
 *  Salin file ini menjadi "Secrets.h" lalu isi nilainya.
 *  Secrets.h sudah masuk .gitignore sehingga password WiFi
 *  dan API key tidak akan ikut ter-push ke GitHub.
 * ============================================================
 */

#ifndef SECRETS_H
#define SECRETS_H

#define WIFI_SSID      "YOUR_WIFI_SSID"      // Nama jaringan WiFi (case-sensitive)
#define WIFI_PASSWORD  "YOUR_WIFI_PASSWORD"  // Password WiFi

//  API key untuk endpoint yang mengubah state (POST /api/restart).
//  Kirim lewat header:  X-API-Key: <API_KEY>
//  Biarkan kosong ("") untuk menonaktifkan endpoint tersebut.
//  Gunakan string acak minimal 16 karakter.
#define API_KEY        ""

#endif // SECRETS_H
