#include "http_server_app.h"

static const char *TAG = "HTTP_Server";
static httpd_handle_t http_server = NULL;

extern const uint8_t image_html_start[] asm("_binary_PIF_png_start");
extern const uint8_t image_html_end[] asm("_binary_PIF_png_end");
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");


static esp_err_t dht11_handler(httpd_req_t *req)
{
    // const char* resp_str = (const char*) "Hello World";
    // httpd_resp_send(req, resp_str, strlen(resp_str));
    // httpd_resp_set_type(req, "image/png");
    // httpd_resp_send(req, (const char*)image_html_start, image_html_end - image_html_start);

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
    return ESP_OK;
}

static const httpd_uri_t get_dht11 = {
    .uri = "/dht11",
    .method = HTTP_GET,
    .handler = dht11_handler,
    .user_ctx = NULL
};


static esp_err_t get_data_dht11_handler(httpd_req_t *req)
{
    const char* resp_str = (const char*) "{\"temperature\": \"25\",\"humidity\": \"80\"}";
    httpd_resp_send(req, resp_str, strlen(resp_str));

    return ESP_OK;
}

static const httpd_uri_t get_data_dht11 = {
    .uri = "/getdatadht11",
    .method = HTTP_GET,
    .handler = get_data_dht11_handler,
    .user_ctx = NULL
};


static esp_err_t sw1_post_handler(httpd_req_t *req)
{
    char buf[100];
    httpd_req_recv(req, buf, req->content_len); 
    printf("SW1 change state: %s\n", buf);

    return ESP_OK;
}

static const httpd_uri_t sw1_post_data = {
    .uri = "/switch1",
    .method = HTTP_POST,
    .handler = sw1_post_handler,
    .user_ctx = NULL
};

static esp_err_t data_post_handler(httpd_req_t *req)
{
    char buf[100];
    // Read the data for the request
    httpd_req_recv(req, buf, req->content_len);
    printf("DATA: %s\n", buf);

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t post_data = {
    .uri = "/data",
    .method = HTTP_POST,
    .handler = data_post_handler,
    .user_ctx = NULL
};

esp_err_t http_404_error_handler_app (httpd_req_t *req, httpd_err_code_t err)
{
    if(strcmp("/dht11", req->uri) == 0)
    {
        httpd_resp_send_err (req, HTTPD_404_NOT_FOUND, "DHT11 URI IS NOT AVAILABLE");
        return ESP_OK;
    }
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "404 NOT FOUND");
    return ESP_FAIL;
}

void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port); 
    if (httpd_start(&http_server, &config) == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(http_server, &post_data);
        httpd_register_uri_handler(http_server, &get_dht11);
        httpd_register_uri_handler(http_server, &sw1_post_data);
        httpd_register_uri_handler(http_server, &get_data_dht11);
        httpd_register_err_handler(http_server, HTTPD_404_NOT_FOUND, http_404_error_handler_app);
    }else
    {
        ESP_LOGI(TAG, "Error starting server!");
    }
}

void stop_webserver(void)
{
    //Stop the http server
    httpd_stop(http_server);
}
