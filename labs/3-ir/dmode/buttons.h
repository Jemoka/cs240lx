#ifndef __BUTTONS_H__
#define __BUTTONS_H__

typedef enum {
    CHM = 16753245,
    CH = 16736925,
    CHP = 16769565,
    LEFT = 16720605,
    RIGHT = 16712445,
    PP = 16761405,
    MINUS = 16769055,
    PLUS = 16754775,
    EQ = 16748655,
    ZERO = 16738455,
    HUNDREDPLUS = 16750695,
    TWODREDPLUS = 16756815,
    ONE = 16724175,
    TWO = 16718055,
    THREE = 16743045,
    FOUR = 16716015,
    FIVE = 16726215,
    SIX = 16734885,
    SEVEN = 16728765,
    EIGHT = 16730805,
    NINE = 16732845
} Buttons;

static inline const char* button_to_str(Buttons b) {
    switch(b) {
    case CHM:
        return "CH-";
    case CH:
        return "CH";
    case CHP:
        return "CH+";
    case LEFT:
        return "LEFT";
    case RIGHT:
        return "RIGHT";
    case PP:
        return "PP";
    case MINUS:
        return "-";
    case PLUS:
        return "+";
    case EQ:
        return "EQ";
    case ZERO:
        return "0";
    case HUNDREDPLUS:
        return "100+";
    case TWODREDPLUS:
        return "200+";
    case ONE:
        return "1";
    case TWO:
        return "2";
    case THREE:
        return "3";
    case FOUR:
        return "4";
    case FIVE:
        return "5";
    case SIX:
        return "6";
    case SEVEN:
        return "7";
    case EIGHT:
        return "8";
    case NINE:
        return "9";
    default:
        return "[noise]";
    }
}


#endif
