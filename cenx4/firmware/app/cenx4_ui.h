#ifndef CENX4_UI_H_
#define CENX4_UI_H_

#include "phi_lib/phi_lib.h"

#ifndef CENX4_UI_EXPOSE_INTERFACES_ONLY
#include "gfx.h"
#else
typedef int16_t coord_t;
typedef void * GDisplay;
#endif

#define CENX4_UI_MAX_LINE_TEXT_LEN		16
#define CENX4_UI_NUM_DISPS              2

/* CENX4_UI_DISPMODE_TEXTS: per-line flags */
#define CENX4_UI_DISPMODE_TEXT_FLAGS_FONT_AUTO  	(0 << 0)
#define CENX4_UI_DISPMODE_TEXT_FLAGS_FONT_SMALL 	(1 << 0)
#define CENX4_UI_DISPMODE_TEXT_FLAGS_FONT_MED   	(2 << 0)
#define CENX4_UI_DISPMODE_TEXT_FLAGS_FONT_LARGE		(3 << 0)

#define CENX4_UI_DISPMODE_TEXT_FLAGS_ALIGN_LEFT   	(0 << 2)
#define CENX4_UI_DISPMODE_TEXT_FLAGS_ALIGN_CENTER 	(1 << 2)
#define CENX4_UI_DISPMODE_TEXT_FLAGS_ALIGN_RIGHT  	(2 << 2)

#define CENX4_UI_DISPMODE_TEXT_FLAGS_BOTTOM       	(1 << 4)

/* CENX4_UI_DISPMODE_SPLIT_POT: global flags */
#define CENX4_UI_DISPMODE_POT_FLAGS_DIVIDER_LINE	(1 << 0)

/* CENX4_UI_DISPMODE_SPLIT_POT: per-pot flags */
#define CENX4_UI_DISPMODE_POT_FLAGS_CENTERED     	(1 << 0)
#define CENX4_UI_DISPMODE_POT_FLAGS_RENDER_VAL   	(1 << 1)
#define CENX4_UI_DISPMODE_POT_FLAGS_MERGE_BOTTOM 	(1 << 2)
#define CENX4_UI_DISPMODE_POT_FLAGS_ROUND		  	(1 << 3)
#define CENX4_UI_DISPMODE_POT_FLAGS_FILL         	(1 << 4)

#define CENX4_UI_DISPMODE_POT_FLAGS_TOP_FONT_AUTO  	(0 << 5)
#define CENX4_UI_DISPMODE_POT_FLAGS_TOP_FONT_SMALL	(1 << 5)
#define CENX4_UI_DISPMODE_POT_FLAGS_TOP_FONT_MED   	(2 << 5)
#define CENX4_UI_DISPMODE_POT_FLAGS_TOP_FONT_LARGE 	(3 << 5)


typedef enum cenx4_ui_dispmode_e
{
	CENX4_UI_DISPMODE_BOOT = 0,
	CENX4_UI_DISPMODE_TEXTS,
	CENX4_UI_DISPMODE_SPLIT_POT,
	CENX4_UI_DISPMODE_LOGO,
	CENX4_UI_NUM_DISPMODES,
} cenx4_ui_dispmode_t;

#pragma pack(1)
typedef union cenx4_ui_dispmode_state_u
{
	struct
	{
		char misc_text[CENX4_UI_MAX_LINE_TEXT_LEN];
	} boot;

	#define CENX4_UI_DISPMODE_TEXTS_LINES 4
	struct
	{
		uint8_t flags[CENX4_UI_DISPMODE_TEXTS_LINES];
		char lines[CENX4_UI_DISPMODE_TEXTS_LINES][CENX4_UI_MAX_LINE_TEXT_LEN];
	} text;

	struct
	{
		struct cenx4_ui_dispmode_state_split_pot_s {
			uint16_t flags;
			char text_top[CENX4_UI_MAX_LINE_TEXT_LEN];
			uint8_t val; // 0 - 99
			char text_bottom[CENX4_UI_MAX_LINE_TEXT_LEN];
		} pots[2];

		uint8_t flags;
	} split_pot;
} cenx4_ui_dispmode_state_t;
#pragma pack()


typedef struct cenx4_ui_s
{
	cenx4_ui_dispmode_t       dispmode;
	cenx4_ui_dispmode_state_t state;
	GDisplay *             g;
	coord_t                w, h;
	mutex_t                lock;
} cenx4_ui_t;



void cenx4_ui_init_obj(cenx4_ui_t * ui, GDisplay * g);
void cenx4_ui_init(void);
cenx4_ui_t * cenx4_ui_lock(uint8_t idx);
void cenx4_ui_unlock(cenx4_ui_t * ui);

void cenx4_ui_render_boot(cenx4_ui_t * ui);
void cenx4_ui_render_texts(cenx4_ui_t * ui);
void cenx4_ui_render_split_pot(cenx4_ui_t * ui);
uint8_t cenx4_ui_get_biggest_possible_font_idx(cenx4_ui_t * ui, const char * s);

void cenx4_ui_render_logo(cenx4_ui_t * ui);

#endif /* CENX4_UI_H_ */
