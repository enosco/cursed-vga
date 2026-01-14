#include <stdlib.h>
#include "ncurses.h"
#include "menu.h"
#include "form.h"
#include "string.h"

typedef struct {
    int size;
    char* choices[32];
} CHOICES;

typedef struct {
    FORM* form;
    WINDOW* win;
    char* win_title;
} FORM_BUNDLE;

typedef struct {
    MENU* menu;
    ITEM* selected_item;
    WINDOW* win;
    char* win_title;
} MENU_BUNDLE;

static const int ACTIVE_COLOR_PAIR = 2;
static const int INVALID_COLOR_PAIR = 3;

ITEM** init_items(CHOICES* choices)
{
    int size = choices->size;    
 
    ITEM** items = (ITEM**) calloc(size+1, sizeof(ITEM*));
    
    for (int i = 0; i < size ; i++) {
        items[i] = new_item(choices->choices[i], NULL);
    }
    items[size] = (ITEM*) NULL;    
    
    return items;
}


void attach_title_to_win(WINDOW* win, char* text)
{
    int y,x;
    getmaxyx(win, y, x);

    int midpoint = (x >> 1) - (strlen(text) >> 1);
    wattron(win, A_BOLD);
    mvwprintw(win, 0, midpoint, "%s", text);
    wattroff(win, A_BOLD);
}

void set_menu_bundle_active(MENU_BUNDLE* bundle)
{
    wattrset(bundle->win, COLOR_PAIR(ACTIVE_COLOR_PAIR));
    box(bundle->win, 0, 0);
    attach_title_to_win(bundle->win, bundle->win_title);
    set_menu_fore(bundle->menu, COLOR_PAIR(ACTIVE_COLOR_PAIR) | A_REVERSE);
    set_menu_back(bundle->menu, COLOR_PAIR(ACTIVE_COLOR_PAIR));
    set_menu_grey(bundle->menu, COLOR_PAIR(INVALID_COLOR_PAIR));
    set_menu_grey(bundle->menu, COLOR_PAIR(INVALID_COLOR_PAIR));
    wrefresh(bundle->win);
}

void set_menu_bundle_inactive(MENU_BUNDLE* bundle)
{
    wattrset(bundle->win, COLOR_PAIR(ACTIVE_COLOR_PAIR) | A_DIM);
    box(bundle->win, 0, 0);
    attach_title_to_win(bundle->win, bundle->win_title);
    set_menu_fore(bundle->menu, COLOR_PAIR(ACTIVE_COLOR_PAIR) | A_REVERSE | A_DIM);
    set_menu_back(bundle->menu, COLOR_PAIR(ACTIVE_COLOR_PAIR) | A_DIM);
    set_menu_grey(bundle->menu, COLOR_PAIR(INVALID_COLOR_PAIR) | A_DIM);
    wrefresh(bundle->win);
}

void set_form_bundle_active(FORM_BUNDLE* bundle)
{
    wattrset(bundle->win, COLOR_PAIR(ACTIVE_COLOR_PAIR));
    box(bundle->win, 0, 0);
    attach_title_to_win(bundle->win, bundle->win_title);
    set_field_back(current_field(bundle->form), A_STANDOUT);    
    wrefresh(bundle->win);
}

void set_form_bundle_inactive(FORM_BUNDLE* bundle)
{
    wattrset(bundle->win, COLOR_PAIR(ACTIVE_COLOR_PAIR) | A_DIM);
    box(bundle->win, 0, 0);
    attach_title_to_win(bundle->win, bundle->win_title);
    set_field_back(current_field(bundle->form), A_STANDOUT | A_DIM);
    wrefresh(bundle->win);
}

void assemble_UI(FORM_BUNDLE* fpath_form_bundle, MENU_BUNDLE* palette_bundle, MENU_BUNDLE* render_bundle)
{
    int UI_frame_w = 51;
    int UI_frame_h = 16;
        
    WINDOW* UI_frame = newwin(UI_frame_h, UI_frame_w, 0, 0);
    
    /*** Init form ***/
    WINDOW* form_frame;
    FORM* fpath_form;
    // Create window for form to reside
    int form_frame_w = UI_frame_w;
    int form_frame_h = 3;
    char* form_win_title = " Path to image ";
    
    form_frame = derwin(UI_frame, form_frame_h, form_frame_w, 0, 0);
    box(form_frame, 0, 0);
    attach_title_to_win(form_frame, form_win_title);
    keypad(form_frame, TRUE);

    char* form_label = "filepath:";    
    mvwprintw(form_frame, 1, 1, "%s", form_label);
    
    // Create form   
    FIELD* fields[2];
    
    int field_width = form_frame_w - strlen(form_label) - 2;
    fields[0] = new_field(1, field_width, 0, 0, 0, 0);
    set_field_back(fields[0], A_STANDOUT);
    
    fields[1] = NULL;
    
    fpath_form = new_form(fields);

    // Attach form to window
    set_form_win(fpath_form, form_frame);
    set_form_sub(fpath_form, derwin(form_frame, 1, field_width, 1, strlen(form_label)+1));
    post_form(fpath_form);
    
    fpath_form_bundle->form = fpath_form;
    fpath_form_bundle->win = form_frame;
    fpath_form_bundle->win_title = form_win_title;
    
    /*** Init menus ***/
    
    // Define choices for each menu
    CHOICES palette_choices = (CHOICES) {
        .size = 4,
        .choices = {
            "Terminal Default",
            "True Color",
            "Ada's 8-bit Color",
            "4-bit Color"
        }
    };

    CHOICES display_choices = (CHOICES) {
        .size = 2,
        .choices = {
            "Default",
            "LED Sim",
        }
    };

    // Create windows for menus to reside
    char* palette_label = " Color palette ";
    char* render_label = " Render mode ";

    int plabel_size = strlen(palette_label);
    int rlabel_size = strlen(render_label);

    // scale palette menu width to leave 1.5x the size needed for the render menu leftover
    int pal_menu_w = plabel_size + (UI_frame_w - plabel_size - rlabel_size*1.5);
    int pal_menu_h = palette_choices.size + 2;
    int disp_menu_w = UI_frame_w - pal_menu_w;
    int disp_menu_h = display_choices.size + 2;

    WINDOW* pal_menu_win = derwin(UI_frame, pal_menu_h, pal_menu_w, form_frame_h, 0);
    WINDOW* disp_menu_win = derwin(UI_frame, disp_menu_h, disp_menu_w, form_frame_h, pal_menu_w);
  
    box(pal_menu_win, 0, 0);
    attach_title_to_win(pal_menu_win, palette_label);

    box(disp_menu_win, 0, 0);
    attach_title_to_win(disp_menu_win, render_label);    

    keypad(pal_menu_win, TRUE);
    keypad(disp_menu_win, TRUE);    

    // Attach items
    ITEM** pal_menu_items = init_items(&palette_choices);
    item_opts_off(pal_menu_items[1], O_SELECTABLE);
    
    ITEM** disp_menu_items = init_items(&display_choices);
    
    // Create menus
    MENU* pal_menu = new_menu(pal_menu_items);   
    MENU* disp_menu = new_menu(disp_menu_items);
    refresh();

    // Attach menus to windows
    set_menu_win(pal_menu, pal_menu_win);
    set_menu_sub(pal_menu, derwin(pal_menu_win, palette_choices.size, pal_menu_w - 2, 1, 1));

    set_menu_win(disp_menu, disp_menu_win);
    set_menu_sub(disp_menu, derwin(disp_menu_win, display_choices.size, disp_menu_w - 2, 1, 1));

    post_menu(pal_menu);
    post_menu(disp_menu);
    
    palette_bundle->menu = pal_menu;
    palette_bundle->selected_item = malloc(sizeof(ITEM*));
    palette_bundle->win = pal_menu_win;
    palette_bundle->win_title = palette_label;

    render_bundle->menu = disp_menu;
    render_bundle->win = disp_menu_win;
    render_bundle->win_title = render_label;    
}


char* get_form_input(FORM_BUNDLE* form_bundle)
{
    getchar();
    int c = ' ';
    while((c = wgetch(form_bundle->win)) != 10) { // 10: Enter key pressed        
        switch(c) {
            case KEY_BACKSPACE: // Backspace
                form_driver(form_bundle->form, REQ_LEFT_CHAR);
                form_driver(form_bundle->form, REQ_DEL_CHAR);
                break;
            default:
                form_driver(form_bundle->form, c);
                break;
        }
    }

    form_driver(form_bundle->form, REQ_VALIDATION);
    return field_buffer(current_field(form_bundle->form), 0);
}


void handle_menus(MENU_BUNDLE* palette_bundle, MENU_BUNDLE* render_bundle)
{
    int active_menu_index = 0;
    MENU_BUNDLE* bundles[2] = {
        palette_bundle,
        render_bundle
    };
    
    int done = 0;
    int c = 0;
    do {
        switch(c)
        {
            case KEY_DOWN:
                menu_driver(bundles[active_menu_index]->menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(bundles[active_menu_index]->menu, REQ_UP_ITEM);
                break;
            case KEY_LEFT:
                if (active_menu_index > 0) {
                    active_menu_index--;
                }                
                break;
            case KEY_RIGHT:
                if (active_menu_index < 1) {
                    active_menu_index++;
                }
                break;
            case 10:
                done = 1;
                break;
        }
        
        for (int index = 0; index < 2; index++) {
            MENU_BUNDLE* curr = bundles[index];
            
            if (index == active_menu_index) {
                wattrset(curr->win, COLOR_PAIR(ACTIVE_COLOR_PAIR));
                set_menu_fore(curr->menu, COLOR_PAIR(ACTIVE_COLOR_PAIR) | A_BLINK | A_REVERSE);
                set_menu_back(curr->menu, COLOR_PAIR(ACTIVE_COLOR_PAIR));
                set_menu_grey(curr->menu, COLOR_PAIR(INVALID_COLOR_PAIR));
            } else {
                wattrset(curr->win, COLOR_PAIR(ACTIVE_COLOR_PAIR) | A_DIM);
                set_menu_fore(curr->menu, COLOR_PAIR(ACTIVE_COLOR_PAIR) | A_REVERSE | A_DIM);
                set_menu_back(curr->menu, COLOR_PAIR(ACTIVE_COLOR_PAIR) | A_DIM);
                set_menu_grey(curr->menu, COLOR_PAIR(INVALID_COLOR_PAIR) | A_DIM);
            }
                        
            box(curr->win, 0, 0);
            wrefresh(curr->win);
        }

    } while(done != 1 && (c = wgetch(bundles[active_menu_index]->win)) != KEY_F(1));
}



// Return 0: item selected and ready to access
// 1: navigated left
// 2: navigated right
// -1: returned to form
int navigate_menu(MENU_BUNDLE* menu_bundle)
{
    int done = 0;
    int c = 0;

    do {
        switch(c) {
            case KEY_DOWN:
                menu_driver(menu_bundle->menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(menu_bundle->menu, REQ_UP_ITEM);
                break;
            case KEY_LEFT:
                return 1;
            case KEY_RIGHT:
                return 2;
            case KEY_ENTER:
            case 10:
                ITEM* curr = current_item(menu_bundle->menu);
                if (item_opts(curr) & O_SELECTABLE) {
                    menu_bundle->selected_item = curr;
                    return 0;
                }
                break;
            case KEY_F(1):
                return -1;
        }
        wrefresh(menu_bundle->win);
    } while(c = wgetch(menu_bundle->win));
}

void navigate_UI(FORM_BUNDLE* filepath_bundle, MENU_BUNDLE* palette_bundle, MENU_BUNDLE* render_bundle)
{
    int done = 0;
    int editing_form = 1;


    wrefresh(filepath_bundle->win);
    wrefresh(palette_bundle->win);
    post_menu(palette_bundle->menu);
    wrefresh(render_bundle->win);
    post_menu(render_bundle->menu);

    int active_menu_index = 0;    

    MENU_BUNDLE* bundles[2] = {
        palette_bundle,
        render_bundle
    };
    
    char* form_input;
    const char* pmenu_input;
    const char* rmenu_input;   
    do {

        //TODO: find out why menu input does not work
        if (editing_form) {
            set_form_bundle_active(filepath_bundle);
            set_menu_bundle_inactive(palette_bundle);
            set_menu_bundle_inactive(render_bundle);
            
            form_input = get_form_input(filepath_bundle);            
            editing_form = 0;
        } else {
//            set_form_bundle_inactive(filepath_bundle);
            
            int result = navigate_menu(bundles[active_menu_index]);            

            if (result == 1 && active_menu_index > 0) {
                active_menu_index--;
            } else if (result == 2 && active_menu_index < 1) {
                active_menu_index++;
            } else if (result == -1) {
                editing_form = 1;
            }

        }
        
        for (int index = 0; index < 2; index++) {
            MENU_BUNDLE* curr = bundles[index];
            
            if (index == active_menu_index) {
                set_menu_bundle_active(bundles[index]);
            } else {                
                set_menu_bundle_inactive(bundles[index]);
            }
                        
            box(curr->win, 0, 0);
            wrefresh(curr->win);
        }

        pmenu_input = item_name(palette_bundle->selected_item);
        rmenu_input = item_name(render_bundle->selected_item);
        
        mvprintw(2,56,":%s",form_input);
        mvprintw(3,56,":%s",pmenu_input);
        mvprintw(4,56,":%s",rmenu_input);
      
        refresh();

    } while (!done);
}

int main()
{
    initscr();
    cbreak();
    start_color();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    
    /**** Initialize Colors ****/
    int INVALID_GREY = 8;    
    init_color(INVALID_GREY,
               500,
               500,
               500);
    
    init_pair(ACTIVE_COLOR_PAIR, COLOR_WHITE, COLOR_BLACK);
    init_pair(INVALID_COLOR_PAIR, INVALID_GREY, COLOR_BLACK);

    /**** Initialize UI ****/

    FORM_BUNDLE* filepath_bundle = malloc(sizeof(FORM_BUNDLE*)); 
    MENU_BUNDLE* palette_bundle = malloc(sizeof(MENU_BUNDLE*)); 
    MENU_BUNDLE* render_bundle = malloc(sizeof(MENU_BUNDLE*));

    assemble_UI(filepath_bundle, palette_bundle, render_bundle);

    /**** Being Navigation ****/    
    navigate_UI(filepath_bundle, palette_bundle, render_bundle);
    
    endwin();
}
