#ifndef HYPERION_UI_H_
#define HYPERION_UI_H_

#include "phi_lib/phi_lib.h"
#include "gfx.h"


#define HYPERION_UI_MAX_LINE_TEXT_LEN		16

/* HYPERION_UI_DISPMODE_TEXTS: per-line flags */
#define HYPERION_UI_DISPMODE_TEXT_FLAGS_FONT_AUTO  	(0 << 0)
#define HYPERION_UI_DISPMODE_TEXT_FLAGS_FONT_SMALL 	(1 << 0)
#define HYPERION_UI_DISPMODE_TEXT_FLAGS_FONT_MED   	(2 << 0)
#define HYPERION_UI_DISPMODE_TEXT_FLAGS_FONT_LARGE		(3 << 0)

#define HYPERION_UI_DISPMODE_TEXT_FLAGS_ALIGN_LEFT   	(0 << 2)
#define HYPERION_UI_DISPMODE_TEXT_FLAGS_ALIGN_CENTER 	(1 << 2)
#define HYPERION_UI_DISPMODE_TEXT_FLAGS_ALIGN_RIGHT  	(2 << 2)

#define HYPERION_UI_DISPMODE_TEXT_FLAGS_BOTTOM       	(1 << 4)

/* HYPERION_UI_DISPMODE_SPLIT_POT: global flags */
#define HYPERION_UI_DISPMODE_POT_FLAGS_DIVIDER_LINE	(1 << 0)

/* HYPERION_UI_DISPMODE_SPLIT_POT: per-pot flags */
#define HYPERION_UI_DISPMODE_POT_FLAGS_CENTERED     	(1 << 0)
#define HYPERION_UI_DISPMODE_POT_FLAGS_RENDER_VAL   	(1 << 1)
#define HYPERION_UI_DISPMODE_POT_FLAGS_MERGE_BOTTOM 	(1 << 2)
#define HYPERION_UI_DISPMODE_POT_FLAGS_ROUND		  	(1 << 3)
#define HYPERION_UI_DISPMODE_POT_FLAGS_FILL         	(1 << 4)

#define HYPERION_UI_DISPMODE_POT_FLAGS_TOP_FONT_AUTO  	(0 << 5)
#define HYPERION_UI_DISPMODE_POT_FLAGS_TOP_FONT_SMALL	(1 << 5)
#define HYPERION_UI_DISPMODE_POT_FLAGS_TOP_FONT_MED   	(2 << 5)
#define HYPERION_UI_DISPMODE_POT_FLAGS_TOP_FONT_LARGE 	(3 << 5)


typedef enum hyperion_ui_dispmode_e
{
	HYPERION_UI_DISPMODE_BOOT = 0,
	HYPERION_UI_DISPMODE_TEXTS,
	HYPERION_UI_DISPMODE_LOGO,
	HYPERION_UI_DISPMODE_CALLBACK,
	HYPERION_UI_NUM_DISPMODES,
} hyperion_ui_dispmode_t;

struct hyperion_ui_s;

#pragma pack(1)
typedef union hyperion_ui_dispmode_state_u
{
	struct
	{
		char misc_text[HYPERION_UI_MAX_LINE_TEXT_LEN];
	} boot;

	#define HYPERION_UI_DISPMODE_TEXTS_LINES 5
	struct
	{
		uint8_t flags[HYPERION_UI_DISPMODE_TEXTS_LINES];
		char lines[HYPERION_UI_DISPMODE_TEXTS_LINES][HYPERION_UI_MAX_LINE_TEXT_LEN];
	} text;


	struct
	{
		void * ctx;
		void (* func)(struct hyperion_ui_s * ui, void * ctx);
	} callback;
} hyperion_ui_dispmode_state_t;
#pragma pack()


typedef struct hyperion_ui_s
{
	hyperion_ui_dispmode_t       dispmode;
	hyperion_ui_dispmode_state_t state;
	GDisplay *             g;
	coord_t                w, h;
	mutex_t                lock;
} hyperion_ui_t;



void hyperion_ui_init_obj(hyperion_ui_t * ui, GDisplay * g);
void hyperion_ui_init(void);
hyperion_ui_t * hyperion_ui_lock(void);
void hyperion_ui_unlock(hyperion_ui_t * ui);

void hyperion_ui_text(hyperion_ui_t * ui, coord_t x, coord_t y, coord_t w, uint8_t font_idx, uint8_t justify, const char * text);

void hyperion_ui_render_boot(hyperion_ui_t * ui);
void hyperion_ui_render_texts(hyperion_ui_t * ui);
uint8_t hyperion_ui_get_biggest_possible_font_idx(hyperion_ui_t * ui, const char * s);

void hyperion_ui_render_logo(hyperion_ui_t * ui);

void hyperion_ui_render_callback(hyperion_ui_t * ui);

#endif /* HYPERION_UI_H_ */
