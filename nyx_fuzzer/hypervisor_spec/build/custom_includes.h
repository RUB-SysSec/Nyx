#pragma once 

#include <stdint.h>

typedef enum TRBType {
    TRB_RESERVED = 0,
    TR_NORMAL,
    TR_SETUP,
    TR_DATA,
    TR_STATUS,
    TR_ISOCH,
    TR_LINK,
    TR_EVDATA,
    TR_NOOP,
    CR_ENABLE_SLOT,
    CR_DISABLE_SLOT,
    CR_ADDRESS_DEVICE,
    CR_CONFIGURE_ENDPOINT,
    CR_EVALUATE_CONTEXT,
    CR_RESET_ENDPOINT,
    CR_STOP_ENDPOINT,
    CR_SET_TR_DEQUEUE,
    CR_RESET_DEVICE,
    CR_FORCE_EVENT,
    CR_NEGOTIATE_BW,
    CR_SET_LATENCY_TOLERANCE,
    CR_GET_PORT_BANDWIDTH,
    CR_FORCE_HEADER,
    CR_NOOP,
    ER_TRANSFER = 32,
    ER_COMMAND_COMPLETE,
    ER_PORT_STATUS_CHANGE,
    ER_BANDWIDTH_REQUEST,
    ER_DOORBELL,
    ER_HOST_CONTROLLER,
    ER_DEVICE_NOTIFICATION,
    ER_MFINDEX_WRAP,
    /* vendor specific bits */
    CR_VENDOR_NEC_FIRMWARE_REVISION  = 49,
    CR_VENDOR_NEC_CHALLENGE_RESPONSE = 50,
} TRBType;


typedef struct{
        size_t dma_buffer_size;
        uintptr_t dma_buffer_virt_addr;
        uintptr_t dma_buffer_phys_addr;

} hypertrash_context_t;

/** A transfer request block template */
struct xhci_trb_template {
  /** Parameter */
  uint32_t parameter_low;
  uint32_t parameter_high;

  /** Status */
  uint32_t status;
  /** Control */
  uint32_t control;
};

/** A transfer request block */
struct xhci_trb_common {
  /** Reserved */
  uint32_t reserved_a_low;
  uint32_t reserved_a_high;

  /** Reserved */
  uint32_t reserved_b;
  /** Flags */
  uint8_t flags;
  /** Type */
  uint8_t type;
  /** Reserved */
  uint16_t reserved_c;
} __attribute__ (( packed ));


/** Transfer request block cycle bit flag */
#define XHCI_TRB_C 0x01

/** Transfer request block toggle cycle bit flag */
#define XHCI_TRB_TC 0x02

/** Transfer request block chain flag */
#define XHCI_TRB_CH 0x10

/** Transfer request block interrupt on completion flag */
#define XHCI_TRB_IOC 0x20

/** Transfer request block immediate data flag */
#define XHCI_TRB_IDT 0x40

/** Transfer request block type */
#define XHCI_TRB_TYPE(type) ( (type) << 2 )

/** Transfer request block type mask */
#define XHCI_TRB_TYPE_MASK XHCI_TRB_TYPE ( 0x3f )

/** A normal transfer request block */
typedef struct xhci_trb_normal_s {
  /** Data buffer */
  uint32_t data_low;
  uint32_t data_high;

  /** Length */
  uint32_t len;
  /** Flags */
  uint8_t flags;
  /** Type */
  uint8_t type;
  /** Reserved */
  uint16_t reserved;
} __attribute__ (( packed )) xhci_trb_normal;


/** A normal transfer request block */
#define XHCI_TRB_NORMAL XHCI_TRB_TYPE ( 1 )

/** Construct TD size field */
#define XHCI_TD_SIZE(remaining) \
  ( ( ( (remaining) <= 0xf ) ? remaining : 0xf ) << 17 )

/** A USB setup data packet */
struct usb_setup_packet {
	/** Request */
	uint16_t request;
	/** Value parameter */
	uint16_t value;
	/** Index parameter */
	uint16_t index;
	/** Length of data stage */
	uint16_t len;
} __attribute__ (( packed ));

/** A setup stage transfer request block */
struct xhci_trb_setup {
  /** Setup packet */
  struct usb_setup_packet packet;
  /** Length */
  uint32_t len;
  /** Flags */
  uint8_t flags;
  /** Type */
  uint8_t type;
  /** Transfer direction */
  uint8_t direction;
  /** Reserved */
  uint8_t reserved;
} __attribute__ (( packed ));

/** A setup stage transfer request block */
#define XHCI_TRB_SETUP XHCI_TRB_TYPE ( 2 )

/** Setup stage input data direction */
#define XHCI_SETUP_IN 3

/** Setup stage output data direction */
#define XHCI_SETUP_OUT 2


/** A data stage transfer request block */
struct xhci_trb_data {
  /** Data buffer */
  uint32_t data_low;
  uint32_t data_high;

  /** Length */
  uint32_t len;
  /** Flags */
  uint8_t flags;
  /** Type */
  uint8_t type;
  /** Transfer direction */
  uint8_t direction;
  /** Reserved */
  uint8_t reserved;
} __attribute__ (( packed ));

/** A data stage transfer request block */
#define XHCI_TRB_DATA XHCI_TRB_TYPE ( 3 )

/** Input data direction */
#define XHCI_DATA_IN 0x01

/** Output data direction */
#define XHCI_DATA_OUT 0x00


/** A status stage transfer request block */
struct xhci_trb_status {
  /** Reserved */
  uint32_t reserved_a_low;
  uint32_t reserved_a_high;

  /** Reserved */
  uint32_t reserved_b;
  /** Flags */
  uint8_t flags;
  /** Type */
  uint8_t type;
  /** Direction */
  uint8_t direction;
  /** Reserved */
  uint8_t reserved_c;
} __attribute__ (( packed ));

/** A status stage transfer request block */
#define XHCI_TRB_STATUS XHCI_TRB_TYPE ( 4 )

/** Input status direction */
#define XHCI_STATUS_IN 0x01

/** Output status direction */
#define XHCI_STATUS_OUT 0x00


/** A link transfer request block */
struct xhci_trb_link {
  /** Next ring segment */
  uint32_t next_low;
  uint32_t next_high;

  /** Reserved */
  uint32_t reserved_a;
  /** Flags */
  uint8_t flags;
  /** Type */
  uint8_t type;
  /** Reserved */
  uint16_t reserved_c;
} __attribute__ (( packed ));

/** A link transfer request block */
#define XHCI_TRB_LINK XHCI_TRB_TYPE ( 6 )

/** A no-op transfer request block */
#define XHCI_TRB_NOP XHCI_TRB_TYPE ( 8 )

/** An enable slot transfer request block */
struct xhci_trb_enable_slot {
  /** Reserved */
  uint32_t reserved_a_low;
  uint32_t reserved_a_high;

  /** Reserved */
  uint32_t reserved_b;
  /** Flags */
  uint8_t flags;
  /** Type */
  uint8_t type;
  /** Slot type */
  uint8_t slot;
  /** Reserved */
  uint8_t reserved_c;
} __attribute__ (( packed ));


/** An enable slot transfer request block */
#define XHCI_TRB_ENABLE_SLOT XHCI_TRB_TYPE ( 9 )

/** A disable slot transfer request block */
struct xhci_trb_disable_slot {
  /** Reserved */
  uint32_t reserved_a_low;
  uint32_t reserved_a_high;

  /** Reserved */
  uint32_t reserved_b;
  /** Flags */
  uint8_t flags;
  /** Type */
  uint8_t type;
  /** Reserved */
  uint8_t reserved_c;
  /** Slot ID */
  uint8_t slot;
} __attribute__ (( packed ));

/** A disable slot transfer request block */
#define XHCI_TRB_DISABLE_SLOT XHCI_TRB_TYPE ( 10 )

/** A context transfer request block */
struct xhci_trb_context {
  /** Input context */
  uint32_t input_low;
  uint32_t input_high;

  /** Reserved */
  uint32_t reserved_a;
  /** Flags */
  uint8_t flags;
  /** Type */
  uint8_t type;
  /** Reserved */
  uint8_t reserved_b;
  /** Slot ID */
  uint8_t slot;
} __attribute__ (( packed ));


/** An address device transfer request block */
#define XHCI_TRB_ADDRESS_DEVICE XHCI_TRB_TYPE ( 11 )

/** A configure endpoint transfer request block */
#define XHCI_TRB_CONFIGURE_ENDPOINT XHCI_TRB_TYPE ( 12 )

/** An evaluate context transfer request block */
#define XHCI_TRB_EVALUATE_CONTEXT XHCI_TRB_TYPE ( 13 )

/** A reset endpoint transfer request block */
struct xhci_trb_reset_endpoint {
  /** Reserved */
  uint32_t reserved_a_low;
  uint32_t reserved_a_high;

  /** Reserved */
  uint32_t reserved_b;
  /** Flags */
  uint8_t flags;
  /** Type */
  uint8_t type;
  /** Endpoint ID */
  uint8_t endpoint;
  /** Slot ID */
  uint8_t slot;
} __attribute__ (( packed ));

/** A reset endpoint transfer request block */
#define XHCI_TRB_RESET_ENDPOINT XHCI_TRB_TYPE ( 14 )


/** A stop endpoint transfer request block */
struct xhci_trb_stop_endpoint {
  /** Reserved */
  uint32_t reserved_a_low;
  uint32_t reserved_a_high;

  /** Reserved */
  uint32_t reserved_b;
  /** Flags */
  uint8_t flags;
  /** Type */
  uint8_t type;
  /** Endpoint ID */
  uint8_t endpoint;
  /** Slot ID */
  uint8_t slot;
} __attribute__ (( packed ));

/** A stop endpoint transfer request block */
#define XHCI_TRB_STOP_ENDPOINT XHCI_TRB_TYPE ( 15 )

/** A set transfer ring dequeue pointer transfer request block */
struct xhci_trb_set_tr_dequeue_pointer {
  /** Dequeue pointer */
  uint32_t dequeue_low;
  uint32_t dequeue_high;

  /** Reserved */
  uint32_t reserved;
  /** Flags */
  uint8_t flags;
  /** Type */
  uint8_t type;
  /** Endpoint ID */
  uint8_t endpoint;
  /** Slot ID */
  uint8_t slot;
} __attribute__ (( packed ));

/** A set transfer ring dequeue pointer transfer request block */
#define XHCI_TRB_SET_TR_DEQUEUE_POINTER XHCI_TRB_TYPE ( 16 )

/** A no-op command transfer request block */
#define XHCI_TRB_NOP_CMD XHCI_TRB_TYPE ( 23 )


/** A transfer event transfer request block */
struct xhci_trb_transfer {
  /** Transfer TRB pointer */
  uint32_t transfer_low;
  uint32_t transfer_high;

  /** Residual transfer length */
  uint16_t residual;
  /** Reserved */
  uint8_t reserved;
  /** Completion code */
  uint8_t code;
  /** Flags */
  uint8_t flags;
  /** Type */
  uint8_t type;
  /** Endpoint ID */
  uint8_t endpoint;
  /** Slot ID */
  uint8_t slot;
} __attribute__ (( packed ));

/** A transfer event transfer request block */
#define XHCI_TRB_TRANSFER XHCI_TRB_TYPE ( 32 )

/** A command completion event transfer request block */
struct xhci_trb_complete {
  /** Command TRB pointer */
  uint32_t command_low;
  uint32_t command_high;

  /** Parameter */
  uint8_t parameter[3];
  /** Completion code */
  uint8_t code;
  /** Flags */
  uint8_t flags;
  /** Type */
  uint8_t type;
  /** Virtual function ID */
  uint8_t vf;
  /** Slot ID */
  uint8_t slot;
} __attribute__ (( packed ));


/** A command completion event transfer request block */
#define XHCI_TRB_COMPLETE XHCI_TRB_TYPE ( 33 )

/** xHCI completion codes */
enum xhci_completion_code {
  /** Success */
  XHCI_CMPLT_SUCCESS = 1,
  /** Short packet */
  XHCI_CMPLT_SHORT = 13,
  /** Command ring stopped */
  XHCI_CMPLT_CMD_STOPPED = 24,
};

/** A port status change transfer request block */
struct xhci_trb_port_status {
  /** Reserved */
  uint8_t reserved_a[3];
  /** Port ID */
  uint8_t port;
  /** Reserved */
  uint8_t reserved_b[7];
  /** Completion code */
  uint8_t code;
  /** Flags */
  uint8_t flags;
  /** Type */
  uint8_t type;
  /** Reserved */
  uint16_t reserved_c;
} __attribute__ (( packed ));

/** A port status change transfer request block */
#define XHCI_TRB_PORT_STATUS XHCI_TRB_TYPE ( 34 )


/** A port status change transfer request block */
struct xhci_trb_host_controller {
  /** Reserved */
  uint32_t reserved_a_low;
  uint32_t reserved_a_high;

  /** Reserved */
  uint8_t reserved_b[3];
  /** Completion code */
  uint8_t code;
  /** Flags */
  uint8_t flags;
  /** Type */
  uint8_t type;
  /** Reserved */
  uint16_t reserved_c;
} __attribute__ (( packed ));

/** A port status change transfer request block */
#define XHCI_TRB_HOST_CONTROLLER XHCI_TRB_TYPE ( 37 )



