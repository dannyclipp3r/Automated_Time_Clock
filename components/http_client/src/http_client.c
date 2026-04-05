#include "http_client.h"

#include "esp_http_client.h"
#include "esp_err.h"
#include "esp_log.h"
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif

#include <stdio.h>
#include <string.h>

static const char *TAG = "BREVO";

#define BREVO_API_KEY "YOUR_BREVO_API_KEY_HERE"
#define FROM_EMAIL    "sender@example.com"
#define TO_EMAIL      "recipient@example.com"

static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "Received %d bytes", evt->data_len);
            break;
        default:
            break;
    }
    return ESP_OK;
}

void build_email_body(char *buffer, size_t len, const char *date_str, double weekly_hours)
{
    if (buffer == NULL || len == 0 || date_str == NULL) {
        return;
    }

    snprintf(buffer, len,
        "{"
        "\"sender\":{\"email\":\"%s\"},"
        "\"to\":[{\"email\":\"%s\"}],"
        "\"subject\":\"Weekly Work Report\","
        "\"textContent\":\"Week of: %s\\n\\nTotal: %.2f hours\""
        "}",
        FROM_EMAIL,
        TO_EMAIL,
        date_str,
        weekly_hours
    );
}

void send_email(const char *email_body)
{
    ESP_LOGI(TAG, "ENTERED send_email()");

    if (email_body == NULL) {
        ESP_LOGE(TAG, "email_body is NULL");
        return;
    }

    esp_http_client_config_t config = {
        .url = "https://api.brevo.com/v3/smtp/email",
        .event_handler = http_event_handler,
        .timeout_ms = 15000,
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
        .crt_bundle_attach = esp_crt_bundle_attach,
#endif
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return;
    }

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "api-key", BREVO_API_KEY);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "accept", "application/json");
    esp_http_client_set_post_field(client, email_body, strlen(email_body));

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "Brevo Status = %d", status);
    } else {
        ESP_LOGE(TAG, "Request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}