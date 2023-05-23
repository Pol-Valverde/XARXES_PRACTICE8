#include "utils.h"


std::string key2str(const sf::Keyboard::Key k) {
    std::string ret;
    switch (k) {

    case sf::Keyboard::A:

        ret = "A";
        break;
    case sf::Keyboard::B:

        ret = "B";
        break;
    case sf::Keyboard::C:

        ret = "C";
        break;
    case sf::Keyboard::D:

        ret = "D";
        break;
    case sf::Keyboard::E:

        ret = "E";
        break;
    case sf::Keyboard::F:

        ret = "F";
        break;
    case sf::Keyboard::G:

        ret = "G";
        break;
    case sf::Keyboard::H:

        ret = "H";
        break;
    case sf::Keyboard::I:

        ret = "I";
        break;
    case sf::Keyboard::J:

        ret = "J";
        break;
    case sf::Keyboard::K:

        ret = "K";
        break;
    case sf::Keyboard::L:

        ret = "L";
        break;
    case sf::Keyboard::M:

        ret = "M";
        break;
    case sf::Keyboard::N:

        ret = "N";
        break;
    case sf::Keyboard::O:

        ret = "O";
        break;
    case sf::Keyboard::P:

        ret = "P";
        break;
    case sf::Keyboard::Q:

        ret = "Q";
        break;
    case sf::Keyboard::R:

        ret = "R";
        break;
    case sf::Keyboard::S:

        ret = "S";
        break;
    case sf::Keyboard::T:

        ret = "T";
        break;
    case sf::Keyboard::U:

        ret = "U";
        break;
    case sf::Keyboard::V:

        ret = "V";
        break;
    case sf::Keyboard::W:

        ret = "W";
        break;
    case sf::Keyboard::X:

        ret = "X";
        break;
    case sf::Keyboard::Y:

        ret = "Y";
        break;
    case sf::Keyboard::Z:

        ret = "Z";
        break;
    case sf::Keyboard::Num0:

        ret = "0";
        break;
    case sf::Keyboard::Num1:

        ret = "1";
        break;
    case sf::Keyboard::Num2:

        ret = "2";
        break;
    case sf::Keyboard::Num3:

        ret = "3";
        break;
    case sf::Keyboard::Num4:

        ret = "4";
        break;
    case sf::Keyboard::Num5:

        ret = "5";
        break;
    case sf::Keyboard::Num6:

        ret = "6";
        break;
    case sf::Keyboard::Num7:

        ret = "7";
        break;
    case sf::Keyboard::Num8:

        ret = "8";
        break;
    case sf::Keyboard::Num9:
        ret = "9";
        break;

    case sf::Keyboard::Numpad0:
        ret = "0";
        break;
    case sf::Keyboard::Numpad1:

        ret = "1";
        break;
    case sf::Keyboard::Numpad2:

        ret = "2";
        break;
    case sf::Keyboard::Numpad3:

        ret = "3";
        break;
    case sf::Keyboard::Numpad4:

        ret = "4";
        break;
    case sf::Keyboard::Numpad5:

        ret = "5";
        break;
    case sf::Keyboard::Numpad6:

        ret = "6";
        break;
    case sf::Keyboard::Numpad7:

        ret = "7";
        break;
    case sf::Keyboard::Numpad8:

        ret = "8";
        break;
    case sf::Keyboard::Numpad9:

        ret = "9";
        break;

    default:
        ret = "";
        break;
    }
    return ret;

}