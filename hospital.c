/*
 * Hospital Management System - C Backend (CGI)
 * File: hospital.c
 *
 * Compile:  gcc hospital.c -o hospital.cgi -lsqlite3
 * Place in: /usr/lib/cgi-bin/  (Apache CGI folder)
 *
 * Handles:
 *   GET  /cgi-bin/hospital.cgi?action=list          -> list all patients
 *   GET  /cgi-bin/hospital.cgi?action=search&q=name -> search patients
 *   GET  /cgi-bin/hospital.cgi?action=delete&id=1   -> delete patient
 *   POST /cgi-bin/hospital.cgi                      -> add new patient
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#define DB_PATH "hospital.db"
#define MAX_BUF 1024

/* ── URL decode ─────────────────────────────────────── */
void url_decode(char *dst, const char *src, int max) {
    int i = 0, j = 0;
    char hex[3] = {0};
    while (src[i] && j < max - 1) {
        if (src[i] == '+') {
            dst[j++] = ' ';
            i++;
        } else if (src[i] == '%' && src[i+1] && src[i+2]) {
            hex[0] = src[i+1];
            hex[1] = src[i+2];
            dst[j++] = (char)strtol(hex, NULL, 16);
            i += 3;
        } else {
            dst[j++] = src[i++];
        }
    }
    dst[j] = '\0';
}

/* ── Get query param ────────────────────────────────── */
void get_param(const char *query, const char *key, char *out, int max) {
    char search[64];
    snprintf(search, sizeof(search), "%s=", key);
    const char *p = strstr(query, search);
    if (!p) { out[0] = '\0'; return; }
    p += strlen(search);
    int i = 0;
    while (*p && *p != '&' && i < max - 1) out[i++] = *p++;
    out[i] = '\0';
    char decoded[MAX_BUF];
    url_decode(decoded, out, max);
    strncpy(out, decoded, max);
}

/* ── CORS + JSON headers ────────────────────────────── */
void send_headers() {
    printf("Content-Type: application/json\r\n");
    printf("Access-Control-Allow-Origin: *\r\n");
    printf("Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n");
    printf("Access-Control-Allow-Headers: Content-Type\r\n");
    printf("\r\n");
}

/* ── Escape for JSON ────────────────────────────────── */
void json_escape(const char *in, char *out, int max) {
    int i = 0, j = 0;
    while (in[i] && j < max - 2) {
        if (in[i] == '"')       { out[j++] = '\\'; out[j++] = '"'; }
        else if (in[i] == '\\') { out[j++] = '\\'; out[j++] = '\\'; }
        else if (in[i] == '\n') { out[j++] = '\\'; out[j++] = 'n'; }
        else                    { out[j++] = in[i]; }
        i++;
    }
    out[j] = '\0';
}

/* ── List / Search patients ─────────────────────────── */
void list_patients(sqlite3 *db, const char *search) {
    char sql[512];
    if (search && strlen(search) > 0) {
        snprintf(sql, sizeof(sql),
            "SELECT id,name,age,gender,disease,doctor,ward,admitted,status "
            "FROM patients WHERE name LIKE '%%%s%%' OR disease LIKE '%%%s%%' "
            "OR doctor LIKE '%%%s%%' ORDER BY id DESC;",
            search, search, search);
    } else {
        snprintf(sql, sizeof(sql),
            "SELECT id,name,age,gender,disease,doctor,ward,admitted,status "
            "FROM patients ORDER BY id DESC;");
    }

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("{\"error\":\"%s\"}", sqlite3_errmsg(db));
        return;
    }

    printf("{\"patients\":[");
    int first = 1;
    char esc[MAX_BUF];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) printf(",");
        first = 0;
        json_escape((const char*)sqlite3_column_text(stmt,1), esc, MAX_BUF);
        printf("{\"id\":%d,\"name\":\"%s\",\"age\":%d,\"gender\":\"%s\","
               "\"disease\":\"%s\",\"doctor\":\"%s\",\"ward\":\"%s\","
               "\"admitted\":\"%s\",\"status\":\"%s\"}",
            sqlite3_column_int(stmt,0), esc,
            sqlite3_column_int(stmt,2),
            sqlite3_column_text(stmt,3),
            sqlite3_column_text(stmt,4),
            sqlite3_column_text(stmt,5),
            sqlite3_column_text(stmt,6),
            sqlite3_column_text(stmt,7),
            sqlite3_column_text(stmt,8));
    }
    printf("]}");
    sqlite3_finalize(stmt);
}

/* ── Add patient ────────────────────────────────────── */
void add_patient(sqlite3 *db, const char *body) {
    char name[128], age[8], gender[16], disease[128];
    char doctor[128], ward[64], admitted[32], status[32];

    get_param(body, "name",     name,     sizeof(name));
    get_param(body, "age",      age,      sizeof(age));
    get_param(body, "gender",   gender,   sizeof(gender));
    get_param(body, "disease",  disease,  sizeof(disease));
    get_param(body, "doctor",   doctor,   sizeof(doctor));
    get_param(body, "ward",     ward,     sizeof(ward));
    get_param(body, "admitted", admitted, sizeof(admitted));
    get_param(body, "status",   status,   sizeof(status));

    char sql[1024];
    snprintf(sql, sizeof(sql),
        "INSERT INTO patients (name,age,gender,disease,doctor,ward,admitted,status) "
        "VALUES ('%s',%s,'%s','%s','%s','%s','%s','%s');",
        name, age, gender, disease, doctor, ward, admitted, status);

    char *err = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        printf("{\"success\":false,\"error\":\"%s\"}", err);
        sqlite3_free(err);
    } else {
        printf("{\"success\":true,\"id\":%lld}", sqlite3_last_insert_rowid(db));
    }
}

/* ── Delete patient ─────────────────────────────────── */
void delete_patient(sqlite3 *db, const char *id) {
    char sql[128];
    snprintf(sql, sizeof(sql), "DELETE FROM patients WHERE id=%s;", id);
    char *err = NULL;
    sqlite3_exec(db, sql, NULL, NULL, &err);
    if (err) {
        printf("{\"success\":false,\"error\":\"%s\"}", err);
        sqlite3_free(err);
    } else {
        printf("{\"success\":true}");
    }
}

/* ── Stats ──────────────────────────────────────────── */
void get_stats(sqlite3 *db) {
    int total=0, admitted=0, critical=0, recovered=0;
    sqlite3_stmt *stmt;

    sqlite3_prepare_v2(db,"SELECT COUNT(*) FROM patients;",-1,&stmt,NULL);
    if(sqlite3_step(stmt)==SQLITE_ROW) total=sqlite3_column_int(stmt,0);
    sqlite3_finalize(stmt);

    sqlite3_prepare_v2(db,"SELECT COUNT(*) FROM patients WHERE status='Admitted';",-1,&stmt,NULL);
    if(sqlite3_step(stmt)==SQLITE_ROW) admitted=sqlite3_column_int(stmt,0);
    sqlite3_finalize(stmt);

    sqlite3_prepare_v2(db,"SELECT COUNT(*) FROM patients WHERE status='Critical';",-1,&stmt,NULL);
    if(sqlite3_step(stmt)==SQLITE_ROW) critical=sqlite3_column_int(stmt,0);
    sqlite3_finalize(stmt);

    sqlite3_prepare_v2(db,"SELECT COUNT(*) FROM patients WHERE status='Recovered';",-1,&stmt,NULL);
    if(sqlite3_step(stmt)==SQLITE_ROW) recovered=sqlite3_column_int(stmt,0);
    sqlite3_finalize(stmt);

    printf("{\"total\":%d,\"admitted\":%d,\"critical\":%d,\"recovered\":%d}",
           total, admitted, critical, recovered);
}

/* ── Main ───────────────────────────────────────────── */
int main() {
    send_headers();

    sqlite3 *db;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        printf("{\"error\":\"Cannot open database\"}");
        return 1;
    }

    char *method  = getenv("REQUEST_METHOD");
    char *query   = getenv("QUERY_STRING");
    if (!query)  query  = "";
    if (!method) method = "GET";

    char action[32], search[128], id[16];
    get_param(query, "action", action, sizeof(action));
    get_param(query, "q",      search, sizeof(search));
    get_param(query, "id",     id,     sizeof(id));

    if (strcmp(method, "POST") == 0) {
        /* Read POST body */
        char *len_str = getenv("CONTENT_LENGTH");
        int len = len_str ? atoi(len_str) : 0;
        char body[2048] = {0};
        if (len > 0 && len < (int)sizeof(body)-1)
            fread(body, 1, len, stdin);
        add_patient(db, body);

    } else if (strcmp(action, "delete") == 0) {
        delete_patient(db, id);

    } else if (strcmp(action, "stats") == 0) {
        get_stats(db);

    } else {
        list_patients(db, search);
    }

    sqlite3_close(db);
    return 0;
}
