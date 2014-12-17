
/* Auction dynamics */
#define MAX_OPEN_AUCTIONS 3         /* At the same time, there are at the height 3 tao */
#define MAX_CLIENTS 5               /* The maximum number of clients to spawn */
#define BASE_BID 42                 /* The base bid "price" */
#define MAX_OFFERS 5                /* Maximum bids from agents in a single tao */
#define BID_INCREMENT 5				/* How much increases every time */

/* Resources */
#define MAX_REQUIRED_RESOURCES 32   /* The maximum number of available resources */
#define MAX_RES_NAME_LENGTH 16      /* The maximum length of the resources name */
#define MAX_REQUIRED_RESOURCES 32   /* The maximum number of resource a client can require. */

/* Messages */
#define MAX_SIMPLE_MESS_LENGTH 512  		/* The maximum length of the simple_message message text */
#define AUCTION_READY_MSG "AUCTION_READY" 	/* DELETE */
