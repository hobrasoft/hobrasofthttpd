/**
 * @file
 *
 * @author Petr Bravenec <petr.bravenec@hobrasoft.cz>
 */
#ifndef __GCC_SETTINGS_H__
#define __GCC_SETTINGS_H__

class Test;

#if __GNUC__ == 4 && __GNUC_MINOR__ < 7
#define FRIEND_CLASS_TEST friend class Test;
#else
#define FRIEND_CLASS_TEST friend Test;
#endif

#endif
