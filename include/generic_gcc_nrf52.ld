/* Linker script to configure memory regions. */

/* This is a slightly adjusted copy from the file: 
 *    $NRF5_DIR/config/nrf52832/armgcc/generic_gcc_nrf52.ld 
 */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

INCLUDE "nrf_symbols.ld"

MEMORY
{
  FLASH (rx) : ORIGIN = APPLICATION_START_ADDRESS, LENGTH = APPLICATION_LENGTH
  RAM (rwx) :  ORIGIN = (RAM_END - RAM_BLUENET_IPC_LENGTH - RAM_APPLICATION_AMOUNT), LENGTH = (RAM_APPLICATION_AMOUNT)
  RAM_BLUENET_IPC (rwx): ORIGIN = (RAM_END - RAM_BLUENET_IPC_LENGTH), LENGTH = RAM_BLUENET_IPC_LENGTH
  CORE_BL_RAM (rw) :     ORIGIN = 0x2000fd00, LENGTH = 0x300
  UICR_BOOTADDR (r) :    ORIGIN = 0x10001014, LENGTH = 0x04
  UICR_MBRPARAMADDR (r): ORIGIN = 0x10001018, LENGTH = 0x04
}

SECTIONS
{
}

SECTIONS
{
  . = ALIGN(4);
  .bluenet_ipc_ram (NOLOAD):
  {
    PROVIDE(__start_bluenet_ipc_ram = .);
    KEEP(*(SORT(.bluenet_ipc_ram.*)))
    PROVIDE(__stop_bluenet_ipc_ram = .);
  } > RAM_BLUENET_IPC
}

SECTIONS
{
  . = ALIGN(4);
  .mem_section_dummy_ram :
  {
  }
  .svc_data :
  {
    PROVIDE(__start_svc_data = .);
    KEEP(*(.svc_data))
    PROVIDE(__stop_svc_data = .);
  } > RAM
  .log_dynamic_data :
  {
    PROVIDE(__start_log_dynamic_data = .);
    KEEP(*(SORT(.log_dynamic_data*)))
    PROVIDE(__stop_log_dynamic_data = .);
  } > RAM
  .log_filter_data :
  {
    PROVIDE(__start_log_filter_data = .);
    KEEP(*(SORT(.log_filter_data*)))
    PROVIDE(__stop_log_filter_data = .);
  } > RAM
  .cli_sorted_cmd_ptrs :
  {
    PROVIDE(__start_cli_sorted_cmd_ptrs = .);
    KEEP(*(.cli_sorted_cmd_ptrs))
    PROVIDE(__stop_cli_sorted_cmd_ptrs = .);
  } > RAM
  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM
  .nrf_mesh_ram :
  {
    PROVIDE(__start_nrf_mesh_ram = .);
    KEEP(*(SORT(.nrf_mesh_ram.*)))
    PROVIDE(__stop_nrf_mesh_ram = .);
  } > RAM

} INSERT AFTER .data;

SECTIONS
{
  .mem_section_dummy_rom :
  {
  }
  .sdh_ant_observers :
  {
    PROVIDE(__start_sdh_ant_observers = .);
    KEEP(*(SORT(.sdh_ant_observers*)))
    PROVIDE(__stop_sdh_ant_observers = .);
  } > FLASH
  .sdh_soc_observers :
  {
    PROVIDE(__start_sdh_soc_observers = .);
    KEEP(*(SORT(.sdh_soc_observers*)))
    PROVIDE(__stop_sdh_soc_observers = .);
  } > FLASH
  .sdh_ble_observers :
  {
    PROVIDE(__start_sdh_ble_observers = .);
    KEEP(*(SORT(.sdh_ble_observers*)))
    PROVIDE(__stop_sdh_ble_observers = .);
  } > FLASH
  .sdh_state_observers :
  {
    PROVIDE(__start_sdh_state_observers = .);
    KEEP(*(SORT(.sdh_state_observers*)))
    PROVIDE(__stop_sdh_state_observers = .);
  } > FLASH
  .sdh_stack_observers :
  {
    PROVIDE(__start_sdh_stack_observers = .);
    KEEP(*(SORT(.sdh_stack_observers*)))
    PROVIDE(__stop_sdh_stack_observers = .);
  } > FLASH
  .sdh_req_observers :
  {
    PROVIDE(__start_sdh_req_observers = .);
    KEEP(*(SORT(.sdh_req_observers*)))
    PROVIDE(__stop_sdh_req_observers = .);
  } > FLASH
  .log_const_data :
  {
    PROVIDE(__start_log_const_data = .);
    KEEP(*(SORT(.log_const_data*)))
    PROVIDE(__stop_log_const_data = .);
  } > FLASH
  .log_backends :
  {
    PROVIDE(__start_log_backends = .);
    KEEP(*(SORT(.log_backends*)))
    PROVIDE(__stop_log_backends = .);
  } > FLASH
  .nrf_balloc :
  {
    PROVIDE(__start_nrf_balloc = .);
    KEEP(*(SORT(.nrf_balloc)))
    PROVIDE(__stop_nrf_balloc = .);
  } > FLASH
  .nrf_queue :
  {
    PROVIDE(__start_nrf_queue = .);
    KEEP(*(.nrf_queue))
    PROVIDE(__stop_nrf_queue = .);
  } > FLASH
  .cli_command :
  {
    PROVIDE(__start_cli_command = .);
    KEEP(*(.cli_command))
    PROVIDE(__stop_cli_command = .);
  } > FLASH
  .crypto_data :
  {
    PROVIDE(__start_crypto_data = .);
    KEEP(*(.crypto_data*))
    PROVIDE(__stop_crypto_data = .);
  } > FLASH
  .pwr_mgmt_data :
  {
    PROVIDE(__start_pwr_mgmt_data = .);
    KEEP(*(SORT(.pwr_mgmt_data*)))
    PROVIDE(__stop_pwr_mgmt_data = .);
  } > FLASH
  .nrf_mesh_flash :
  {
    PROVIDE(__start_nrf_mesh_flash = .);
    KEEP(*(SORT(.nrf_mesh_flash.*)))
    PROVIDE(__stop_nrf_mesh_flash = .);
  } > FLASH
} INSERT AFTER .text

INCLUDE "nrf_common.ld"
