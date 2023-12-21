#include <zephyr/kernel.h>
#include <stdio.h>
#include <string.h>
// include LTE link controller & Modem libary
#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>

#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>


//logregister
LOG_MODULE_REGISTER(Simple_tracker, LOG_LEVEL_INF);

//Create semaphore for lte connection
static K_SEM_DEFINE(lte_con, 0, 1);     //K_SEM_DEFINE(name, intial_count, Max_count)

//Create LTE event handler
static void lte_handler(const struct lte_lc_evt *const evt){

        switch (evt->type) {
        
        case LTE_LC_EVT_NW_REG_STATUS:
                 if ((evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_HOME) &&
		     (evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_ROAMING)) {
		        break;
		}
                //release semaphore
                k_sem_give(&lte_con);
                break;
                //get RRC mode status
        case LTE_LC_EVT_RRC_UPDATE:
		LOG_INF("RRC mode: %s", evt->rrc_mode == LTE_LC_RRC_MODE_CONNECTED ?
				"Connected" : "Idle");
		break;
        
        default:
                break;
        }
}

//conf modem libary
static int modem_configure(void){

	int err;
	LOG_INF("Initializing modem library");
	err = nrf_modem_lib_init();
	if (err) {
		LOG_ERR("Failed to initialize the modem library, error: %d", err);
		return err;
	}
	err = lte_lc_init_and_connect_async(lte_handler);
	if (err) {
		LOG_ERR("Modem could not be configured, error: %d", err);
		return err;
	}
	return 0;
}



int main(void)
{
        int err;

        if (dk_leds_init() != 0) {
		LOG_ERR("Failed to initialize the LEDs Library");
	}

        err = modem_configure();
	if (err) {
		LOG_ERR("Failed to configure the modem");
		return 0;
	}
        //take semaphore
        k_sem_take(&lte_con, K_FOREVER);


        return 0;
}
