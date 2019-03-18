/**
 * @file main.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2018-12-25
 *
 * @copyright Copyright (c) 2018
 *
 */

// #include "ic_board.h"

#include "ic_bt.h"
#include "ic_buttons.h"
#include "ic_version.h"

#define SLEEP_TIME 250

#define PERIOD (USEC_PER_SEC / 50)
#define FADESTEP 2000
#define FADESTEP_LEVEL 6553

// Initializing variables
struct k_work pressed_work[BUTTON_NUMBERS];

u16_t level_state               = 0;
u8_t state                      = 0;
ic_buttons_device_t buttons_dev = {0};

struct level_cli led_level_cli[] = {
    {
        .level_state = &level_state,
        .act         = 0,
        .tid         = 0,
    },
    {
        .level_state = &level_state,
        .act         = 0,
        .tid         = 0,
    },
};

struct onoff_cli led_onoff_cli[] = {
    {
        .state = &state,
        .act   = 0,
        .tid   = 0,
    },
    {
        .state = &state,
        .act   = 0,
        .tid   = 0,
    },
};

// Initializing BLE
static const uint8_t dev_uuid[16] = {0xdd, 0xdd};


// Defining the msg publishers
// 2 Bytes for opcode
// 2 Bytes for msg content
// 1 Byte for delay and transient
BT_MESH_MODEL_PUB_DEFINE(pub_onoff_cli_on, NULL, 2 + 2);
BT_MESH_MODEL_PUB_DEFINE(pub_onoff_cli_off, NULL, 2 + 2);
BT_MESH_MODEL_PUB_DEFINE(pub_level_cli_more, NULL, 2 + 2 + 1);
BT_MESH_MODEL_PUB_DEFINE(pub_level_cli_less, NULL, 2 + 2 + 1);


// Defining config server
static struct bt_mesh_cfg_srv cfg_srv = {
    .relay            = BT_MESH_RELAY_DISABLED,
    .beacon           = BT_MESH_BEACON_ENABLED,
    .frnd             = BT_MESH_FRIEND_NOT_SUPPORTED,
    .gatt_proxy       = BT_MESH_GATT_PROXY_ENABLED,
    .default_ttl      = 7,
    .net_transmit     = BT_MESH_TRANSMIT(2, 20),
    .relay_retransmit = BT_MESH_TRANSMIT(2, 20),
};

// Health server model
BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);

static struct bt_mesh_health_srv health_srv = {};
static struct bt_mesh_cfg_cli cfg_cli       = {};

static struct bt_mesh_model less_model[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_LEVEL_CLI, generic_level_cli_op, &pub_level_cli_less,
                  &led_level_cli[0]),
};

static struct bt_mesh_model more_model[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_LEVEL_CLI, generic_level_cli_op, &pub_level_cli_more,
                  &led_level_cli[1]),
};

static struct bt_mesh_model on_model[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, generic_onoff_cli_op, &pub_onoff_cli_on,
                  &led_onoff_cli[0]),
};

static struct bt_mesh_model off_model[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, generic_onoff_cli_op, &pub_onoff_cli_off,
                  &led_onoff_cli[1]),
};

static struct bt_mesh_model root_models[] = {
    BT_MESH_MODEL_CFG_SRV(&cfg_srv),
    BT_MESH_MODEL_CFG_CLI(&cfg_cli),
    BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
};

struct bt_mesh_elem elements[] = {
    BT_MESH_ELEM(0, root_models, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, less_model, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, more_model, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, on_model, BT_MESH_MODEL_NONE),
    BT_MESH_ELEM(0, off_model, BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp comp = {
    .cid        = BT_COMP_ID_LF,
    .elem       = elements,
    .elem_count = ARRAY_SIZE(elements),
};

static int output_number(bt_mesh_output_action_t action, u32_t number);
static void prov_complete(u16_t net_idx, u16_t addr);
static void prov_reset(void);

static const struct bt_mesh_prov prov = {
    .uuid           = dev_uuid,
    .output_size    = 1,
    .output_actions = BT_MESH_DISPLAY_NUMBER,
    .output_number  = output_number,
    .complete       = prov_complete,
    .reset          = prov_reset,
};

static void bt_ready(int err)
{
    if (err) {
        printk("bt_enable init failed with err %d\n", err);
        return;
    }

    printk("Bluetooth initialized.\n");
    err = bt_mesh_init(&prov, &comp);
    if (err) {
        printk("bt_mesh init failed with err %d\n", err);
        return;
    }

    if (IS_ENABLED(CONFIG_SETTINGS)) {
        settings_load();
    }

    /* This will be a no-op if settings_load() loaded provisioning info */
    bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
    printk("Mesh initialized.\n");
}

void ic_buttons_callback(struct device *buttons_device, struct gpio_callback *callback,
                         u32_t button_pin_mask)
{
    time = k_uptime_get_32();

    if (time < last_time + BUTTON_DEBOUNCE_DELAY_MS) {
        last_time = time;
        return;
    }

    if (ic_buttons_pin_to_i(button_pin_mask) == 0) {
        k_work_submit(&led_onoff_cli[0].callback_work);
    } else if (ic_buttons_pin_to_i(button_pin_mask) == 1) {
        k_work_submit(&led_onoff_cli[1].callback_work);
    } else if (ic_buttons_pin_to_i(button_pin_mask) == 2) {
        k_work_submit(&led_level_cli[0].callback_work);
    } else if (ic_buttons_pin_to_i(button_pin_mask) == 3) {
        k_work_submit(&led_level_cli[1].callback_work);
    }
    last_time = time;
}

// Bluetooth mesh initialization

static int output_number(bt_mesh_output_action_t action, u32_t number)
{
    printk("OOB Number: %u\n", number);
    return 0;
}


static void prov_complete(u16_t net_idx, u16_t addr)
{
    printk("Provisioning was completed.\n");
}

static void prov_reset(void)
{
    bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
}

void button_1_pressed(struct k_work *work)
{
    printk("Button 1 pressed\n");

    struct onoff_cli *h = CONTAINER_OF(work, struct onoff_cli, callback_work);
    h->act              = !(*h->state);

    send_generic_onoff_set(h, BT_MESH_MODEL_OP_GENERIC_ONOFF_SET);
}

void button_3_pressed(struct k_work *work)
{
    printk("Button 3 pressed\n");
    struct level_cli *h = CONTAINER_OF(work, struct level_cli, callback_work);

    h->act = *h->level_state - FADESTEP < 0 ? 0 : *h->level_state - FADESTEP;

    send_generic_level_set(h, BT_MESH_MODEL_OP_GENERIC_LEVEL_SET);
}

void button_4_pressed(struct k_work *work)
{
    printk("Button 4 pressed\n");
    struct level_cli *h = CONTAINER_OF(work, struct level_cli, callback_work);

    h->act = *h->level_state + FADESTEP > PERIOD ? PERIOD : *h->level_state + FADESTEP;

    send_generic_level_set(h, BT_MESH_MODEL_OP_GENERIC_LEVEL_SET);
}

int configure_board()
{
    led_onoff_cli[0].callback_work = pressed_work[0];
    led_onoff_cli[0].model_cli     = &on_model[0];

    led_level_cli[0].callback_work = pressed_work[2];
    led_level_cli[0].model_cli     = &less_model[0];

    led_level_cli[1].callback_work = pressed_work[3];
    led_level_cli[1].model_cli     = &more_model[0];

    k_work_init(&led_onoff_cli[0].callback_work, button_1_pressed);
    k_work_init(&led_level_cli[0].callback_work, button_3_pressed);
    k_work_init(&led_level_cli[1].callback_work, button_4_pressed);

    ic_buttons_init_device(&buttons_dev);
    ic_buttons_configure(&buttons_dev);
    ic_buttons_configure_callback(&buttons_dev, ic_buttons_callback);


    return 0;
}

void main(void)
{
    printk("Firmware version: %d.%d.%d\n", ic_version_get_major(), ic_version_get_minor(),
           ic_version_get_build());

    int err;
    err = configure_board();
    if (err) {
        return;
    }

    err = bt_enable(bt_ready);
    if (err) {
        printk("bt_enable failed with err %d\n", err);
        return;
    }
    while (1) {
        k_sleep(SLEEP_TIME);
    }
}