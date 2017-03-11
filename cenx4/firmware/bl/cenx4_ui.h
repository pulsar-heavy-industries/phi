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
#define CENX4_UI_NUM_DISPS             2


typedef enum cenx4_ui_dispmode_e
{
	CENX4_UI_DISPMODE_LOGO = 0,
	CENX4_UI_DISPMODE_BOOT,
} cenx4_ui_dispmode_t;

#pragma pack(1)
typedef union cenx4_ui_dispmode_state_u
{
	struct
	{
		char misc_text[3][CENX4_UI_MAX_LINE_TEXT_LEN];
	} boot;

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

void cenx4_ui_render_logo(cenx4_ui_t * ui);
void cenx4_ui_render_boot(cenx4_ui_t * ui);

#endif /* CENX4_UI_H_ */
