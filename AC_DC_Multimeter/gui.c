/* Provides the commands for the user interface */
/* Serving as a wrapper for vt100.h and vt100.c */

#include "gui.h"

// setup integers for gui dimensions
static int width = 69;
static int height = 25;
static uint8_t mode = DC;

// title block strings
static char *title = "AC / DC DIGITAL MULTIMETER V1";
//static char *mode_dc = "Mode (Press TAB to switch): <DC>  AC ";
//static char *mode_ac = "Mode (Press TAB to switch):  DC  <AC>";
char *modes[2] = {"Mode (Press TAB to switch): <DC>  AC ","Mode (Press TAB to switch):  DC  <AC>"};

// bar graph titles
static char *dc_voltage = "DC Voltage [V]: ";
static char *rms_voltage = "RMS Voltage [V]: ";
static char *vpp_voltage = "Vpp [V]: ";
static char *frequency = "Frequency [Hz]: ";

static uint8_t offset_dc_voltage;
static uint8_t offset_rms_voltage;
static uint8_t offset_vpp_voltage;
static uint8_t offset_frequency;

// offset locations
static uint8_t graph_1_title = 10; 	// 7th row
static uint8_t graph_2_title = 15;	// 12th row
static uint8_t graph_3_title = 20;	// 15th row

static uint8_t graph_x_end = 17;

void gui_init(EUSCI_A_Type *x, uint16_t interrupt_flag) {
	vt100_init(x, interrupt_flag);
	mode = AC;	// default init to DC mode;
	gui_draw_border();
	gui_draw_title();
	gui_set_mode(DC);
	return;
}

void gui_draw_border(void) {
    int i, j;
    vt100_clear_screen();
    vt100_set_cursor(1,2);

    // Draw top border
    for(i = 2; i < width; i++) {
        vt100_write_char('_');
    }

    // Draw horizontal borders
    for (i = 2; i < height + 2; i++) {
        vt100_set_cursor(i, 0);
        vt100_write_char('|');
        vt100_set_cursor(i, width);
        vt100_write_char('|');
    }

	// draw bottom border
    vt100_set_cursor(height + 1,2);
    for(j = 2; j < width; j++) {
        vt100_write_char('_');
    }
	
	// draw title block border
	vt100_set_cursor(8, 2);
	for(j = 2; j < width; j++) {
        vt100_write_char('_');
    }

}

void gui_draw_title(void) {	
	vt100_set_cursor(3,4);
	vt100_write_string(title);
	vt100_set_cursor(6,4);
	gui_set_mode(mode);
}

void gui_set_mode(uint8_t m) {
    if (m == DC || m == AC) {
        if (m != mode) {
            if (m == DC) {
                gui_DC_display();
            } else {
                gui_AC_display();
            }
        }
        mode = m;
        vt100_set_cursor(6,4);
        vt100_write_string(modes[mode]);
    }

}

void gui_graph_display(uint8_t graph_offset, uint8_t filled_buckets) {
    // setup for width of 66;
    uint8_t num_pos = width - 3;
    uint8_t i = 0;

    vt100_set_cursor(graph_offset + 1, 3);
    if (filled_buckets < num_pos) {
        while (i++ < filled_buckets) {
            vt100_write_char('|');
        }
        while (i++ < num_pos) {
            vt100_write_char('-');
        }
    } else {
        while (i++ < num_pos - 1)
            vt100_write_char('|');
    }
}

void gui_DC_display(void) {
    uint8_t i, j;

    vt100_set_cursor(graph_1_title, 3);
    offset_dc_voltage = vt100_write_string(dc_voltage) + 5;

    vt100_set_cursor(graph_1_title +1, 3);
    gui_graph_display(graph_1_title, 0);

    vt100_set_cursor(graph_1_title + 2, 3);
    vt100_write_string("0");
    vt100_set_cursor(graph_1_title + 2, width - 5);
    vt100_write_string("3.3V");

    // clear out the rest from AC Display
    for (i = graph_2_title; i < graph_3_title + 3; i++) {
        vt100_set_cursor(i, 3);
        for (j = 3; j < width - 1; j++) {
            vt100_write_char(' ');
        }
    }
}

void gui_AC_display(void) {
    vt100_set_cursor(graph_1_title, 3);
    offset_rms_voltage = vt100_write_string(rms_voltage) + 4;

    vt100_set_cursor(graph_1_title +1, 3);
    gui_graph_display(graph_1_title, 0);

    vt100_set_cursor(graph_1_title + 2, 3);
    vt100_write_string("0");
    vt100_set_cursor(graph_1_title + 2, width - 5);
    vt100_write_string("3.3V");

    vt100_set_cursor(graph_2_title, 3);
    offset_vpp_voltage = vt100_write_string(vpp_voltage) + 4;

    vt100_set_cursor(graph_2_title +1, 3);
    gui_graph_display(graph_2_title, 0);

    vt100_set_cursor(graph_2_title + 2, 3);
    vt100_write_string("0");
    vt100_set_cursor(graph_2_title + 2, width - 5);
    vt100_write_string("3.3V");

    vt100_set_cursor(graph_3_title, 3);
    offset_frequency = vt100_write_string(frequency) + 4;

    vt100_set_cursor(graph_3_title +1, 3);
    gui_graph_display(graph_3_title, 0);

    vt100_set_cursor(graph_3_title + 2, 3);
    vt100_write_string("0");
    vt100_set_cursor(graph_3_title + 2, width - 7);
    vt100_write_string("1000Hz");
    return;
}

// get mode
uint8_t gui_get_mode() {
    return mode;
}

void gui_DC_update(uint32_t value_dc) {
    uint32_t volt_dc = (value_dc * 1000)/4963;
    uint8_t buckets_dc = (2 * volt_dc)/100;

    // write current voltage to display
    vt100_set_cursor(graph_1_title, offset_dc_voltage);
    vt100_write_string(format_number(volt_dc));

    vt100_set_cursor(graph_1_title +1, 3);
    gui_graph_display(graph_1_title, buckets_dc);

}

char * format_number(int32_t value) {
    char * index;
    char * f_value;
    char buf[10];
    int i, j;

    i = j = 0;

    index = f_value = itoa(value);

    if (value < 1000) {
        vt100_write_string("0.");
    }
    if (value < 100) {
        vt100_write_string("0");
    }
    if (value < 10) {
        vt100_write_string("0");
    }
    // get length
    while (index && *index) {
        index++;
        i++;
    }
    i = i - 3;

    while (f_value && *f_value) {
        buf[j++] = *f_value;
        if (j == i) {
            buf[j++] = '.';
        }
        f_value++;
    }
    buf[j] = '\0';

    return buf;
}
//4961
void gui_AC_update(uint32_t value_rms, uint32_t value_vpp, uint32_t value_freq) {
    uint32_t volt_rms = (value_rms * 1000)/5000;
    uint32_t volt_vpp = (value_vpp * 1000)/5200;
    uint32_t freq = value_freq;

    uint8_t buckets_rms = (2 * volt_rms)/100;
    uint8_t buckets_vpp = (2 * volt_vpp)/100;
    uint8_t buckets_freq = freq / 15000;

    // Update RMS voltage
    vt100_set_cursor(graph_1_title, offset_rms_voltage);

    vt100_write_string(format_number(volt_rms));
    vt100_set_cursor(graph_1_title +1, 3);
    gui_graph_display(graph_1_title, buckets_rms);

    // Update Vpp voltage
    vt100_set_cursor(graph_2_title, offset_vpp_voltage);

    vt100_write_string(format_number(volt_vpp));
    vt100_set_cursor(graph_2_title +1, 3);
    gui_graph_display(graph_2_title, buckets_vpp);

    // Update frequency
    vt100_set_cursor(graph_3_title, offset_frequency);
    if(freq < 1000000) {
        vt100_write_string(" ");
    }

    vt100_write_string(format_number(freq));
    vt100_set_cursor(graph_3_title +1, 3);
    gui_graph_display(graph_3_title, buckets_freq);

}

