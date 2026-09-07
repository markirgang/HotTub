/*****************************************************************************
 * | File         :   i2c.c
 * | Author       :   Waveshare team
 * | Function     :   Hardware underlying interface
 * | Info         :
 * |                 I2C driver code for I2C communication.
 * ----------------
 * | This version :   V1.0
 * | Date         :   2024-11-26
 * | Info         :   Basic version
 *
 ******************************************************************************/

#include "i2c.h"  // Include I2C driver header for I2C functions
static const char *TAG = "i2c";  // Define a tag for logging

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 1, 0)
#define I2C_CLK_SRC_DEFAULT 0
static esp_err_t i2c_new_master_bus(const i2c_master_bus_config_t *bus_config, i2c_master_bus_handle_t *ret_bus) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = bus_config->sda_io_num,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = bus_config->scl_io_num,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    i2c_param_config(bus_config->i2c_port, &conf);
    esp_err_t ret = i2c_driver_install(bus_config->i2c_port, conf.mode, 0, 0, 0);
    *ret_bus = bus_config->i2c_port;
    return ret;
}

static esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t bus, const i2c_device_config_t *dev_config, i2c_master_dev_handle_t *ret_dev) {
    *ret_dev = dev_config->dev_addr;
    return ESP_OK;
}

static esp_err_t i2c_master_transmit(i2c_master_dev_handle_t dev_handle, const uint8_t *write_buffer, size_t write_size, int xfer_timeout_ms) {
    return i2c_master_write_to_device(EXAMPLE_I2C_MASTER_NUM, (uint8_t)dev_handle, write_buffer, write_size, pdMS_TO_TICKS(xfer_timeout_ms < 0 ? 1000 : xfer_timeout_ms));
}

static esp_err_t i2c_master_transmit_receive(i2c_master_dev_handle_t dev_handle, const uint8_t *write_buffer, size_t write_size, uint8_t *read_buffer, size_t read_size, int xfer_timeout_ms) {
    return i2c_master_write_read_device(EXAMPLE_I2C_MASTER_NUM, (uint8_t)dev_handle, write_buffer, write_size, read_buffer, read_size, pdMS_TO_TICKS(xfer_timeout_ms < 0 ? 1000 : xfer_timeout_ms));
}

static esp_err_t i2c_master_receive(i2c_master_dev_handle_t dev_handle, uint8_t *read_buffer, size_t read_size, int xfer_timeout_ms) {
    return i2c_master_read_from_device(EXAMPLE_I2C_MASTER_NUM, (uint8_t)dev_handle, read_buffer, read_size, pdMS_TO_TICKS(xfer_timeout_ms < 0 ? 1000 : xfer_timeout_ms));
}
#endif

// Global handle for the I2C master bus
DEV_I2C_Port handle;
/**
 * @brief Initialize the I2C master interface.
 * 
 * This function configures the I2C master bus and adds a device with the specified address.
 * The I2C clock source, frequency, SCL/SDA pins, and other settings are configured here.
 * 
 * @param Addr The I2C address of the device to be initialized.
 * @return The device handle if initialization is successful, NULL otherwise.
 */
DEV_I2C_Port DEV_I2C_Init()
{
    // Define I2C bus configuration parameters
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,       // Default clock source for I2C
        .i2c_port = EXAMPLE_I2C_MASTER_NUM,     // I2C master port number
        .scl_io_num = EXAMPLE_I2C_MASTER_SCL,   // I2C SCL (clock) pin
        .sda_io_num = EXAMPLE_I2C_MASTER_SDA,   // I2C SDA (data) pin
        .glitch_ignore_cnt = 7,                  // Ignore glitches in the I2C signal
    };

    // Create a new I2C master bus with the above configuration
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &handle.bus));
    
    // Configure the device's I2C parameters
    i2c_device_config_t i2c_dev_conf = {
        .scl_speed_hz = EXAMPLE_I2C_MASTER_FREQUENCY,  // Set I2C communication speed
    };
    
    // Add the I2C device to the bus
    // i2c_master_dev_handle_t dev_handle = NULL;
    if (i2c_master_bus_add_device(handle.bus, &i2c_dev_conf, &handle.dev) != ESP_OK) {
        ESP_LOGE(TAG, "I2C device creation failed");  // Log error if device creation fails
    }

    return handle;  // Return the device handle if successful
}

/**
 * @brief Set a new I2C slave address for the device.
 * 
 * This function allows changing the I2C slave address for the specified device.
 * 
 * @param dev_handle The handle to the I2C device.
 * @param Addr The new I2C address for the device.
 */
void DEV_I2C_Set_Slave_Addr(i2c_master_dev_handle_t *dev_handle, uint8_t Addr)
{
    // Configure the new device address
    i2c_device_config_t i2c_dev_conf = {
        .scl_speed_hz = EXAMPLE_I2C_MASTER_FREQUENCY,  // I2C frequency
        .device_address = Addr,                        // Set new device address
    };
    
    // Update the device with the new address
    if (i2c_master_bus_add_device(handle.bus, &i2c_dev_conf, dev_handle) != ESP_OK) {
        ESP_LOGE(TAG, "I2C address modification failed");  // Log error if address modification fails
    }
}

/**
 * @brief Write a single byte to the I2C device.
 * 
 * This function sends a command byte and a value byte to the I2C device.
 * 
 * @param dev_handle The handle to the I2C device.
 * @param Cmd The command byte to send.
 * @param value The value byte to send.
 */
void DEV_I2C_Write_Byte(i2c_master_dev_handle_t dev_handle, uint8_t Cmd, uint8_t value)
{
    uint8_t data[2] = {Cmd, value};  // Create an array with command and value
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, data, sizeof(data), 100));  // Send the data to the device
}

/**
 * @brief Read a single byte from the I2C device.
 * 
 * This function reads a byte of data from the I2C device.
 * 
 * @param dev_handle The handle to the I2C device.
 * @return The byte read from the device.
 */
uint8_t DEV_I2C_Read_Byte(i2c_master_dev_handle_t dev_handle)
{
    uint8_t data[1] = {0};  // Create a buffer to store the received byte
    ESP_ERROR_CHECK(i2c_master_receive(dev_handle, data, 1, 100));  // Read a byte from the device
    return data[0];  // Return the received byte
}

/**
 * @brief Read a word (2 bytes) from the I2C device.
 * 
 * This function reads two bytes (a word) from the I2C device.
 * The data is received by sending a command byte and receiving the data.
 * 
 * @param dev_handle The handle to the I2C device.
 * @param Cmd The command byte to send.
 * @return The word read from the device (combined two bytes).
 */
uint16_t DEV_I2C_Read_Word(i2c_master_dev_handle_t dev_handle, uint8_t Cmd)
{
    uint8_t data[2] = {Cmd};  // Create an array with the command byte
    ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, data, 1, data, 2, 100));  // Send command and receive two bytes
    return data[1] << 8 | data[0];  // Combine the two bytes into a word (16-bit)
}

/**
 * @brief Write multiple bytes to the I2C device.
 * 
 * This function sends a block of data to the I2C device.
 * 
 * @param dev_handle The handle to the I2C device.
 * @param pdata Pointer to the data to send.
 * @param len The number of bytes to send.
 */
void DEV_I2C_Write_Nbyte(i2c_master_dev_handle_t dev_handle, uint8_t *pdata, uint8_t len)
{
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, pdata, len, 100));  // Transmit the data block
}

/**
 * @brief Read multiple bytes from the I2C device.
 * 
 * This function reads multiple bytes from the I2C device.
 * The function sends a command byte and receives the specified number of bytes.
 * 
 * @param dev_handle The handle to the I2C device.
 * @param Cmd The command byte to send.
 * @param pdata Pointer to the buffer where received data will be stored.
 * @param len The number of bytes to read.
 */
void DEV_I2C_Read_Nbyte(i2c_master_dev_handle_t dev_handle, uint8_t Cmd, uint8_t *pdata, uint8_t len)
{
    ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, &Cmd, 1, pdata, len, 100));  // Send command and receive data
}
