/* ************************************************************************
*   File: act.social.c                                  Part of CircleMUD *
*  Usage: Functions to handle socials                                     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"


/* local globals */
static int list_top = -1;

/* local functions */
char *fread_action(FILE *fl, int nr);
int find_action(int cmd);
ACMD(do_action);
ACMD(do_insult);
void boot_social_messages(void);
void free_social_messages(void);


struct social_messg {
  int act_nr;
  int hide;
  int min_victim_position;	/* Position of victim */

  /* No argument was supplied */
  char *char_no_arg;
  char *others_no_arg;

  /* An argument was there, and a victim was found */
  char *char_found;		/* if NULL, read no further, ignore args */
  char *others_found;
  char *vict_found;

  /* An argument was there, but no victim was found */
  char *not_found;

  /* The victim turned out to be the character */
  char *char_auto;
  char *others_auto;
} *soc_mess_list;


int find_action(int cmd)
{
  int bot, top, mid;

  bot = 0;
  top = list_top;

  if (top < 0)
    return (-1);

  for (;;) {
    mid = (bot + top) / 2;

    if (soc_mess_list[mid].act_nr == cmd)
      return (mid);
    if (bot >= top)
      return (-1);

    if (soc_mess_list[mid].act_nr > cmd)
      top = --mid;
    else
      bot = ++mid;
  }
}



ACMD(do_action)
{
  char buf[MAX_INPUT_LENGTH];
  int act_nr;
  struct social_messg *action;
  struct char_data *vict;

  if ((act_nr = find_action(cmd)) < 0) {
    send_to_char(ch, "That action is not supported.\r\n");
    return;
  }
  action = &soc_mess_list[act_nr];

  if (action->char_found && argument)
    one_argument(argument, buf);
  else
    *buf = '\0';

  if (!*buf) {
    send_to_char(ch, "%s\r\n", action->char_no_arg);
    act(action->others_no_arg, action->hide, ch, 0, 0, TO_ROOM);
    return;
  }
  if (!(vict = get_char_vis(ch, buf, NULL, FIND_CHAR_ROOM)))
    send_to_char(ch, "%s\r\n", action->not_found);
  else if (vict == ch) {
    send_to_char(ch, "%s\r\n", action->char_auto);
    act(action->others_auto, action->hide, ch, 0, 0, TO_ROOM);
  } else {
    if (GET_POS(vict) < action->min_victim_position)
      act("$N is not in a proper position for that.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
    else {
      act(action->char_found, 0, ch, 0, vict, TO_CHAR | TO_SLEEP);
      act(action->others_found, action->hide, ch, 0, vict, TO_NOTVICT);
      act(action->vict_found, action->hide, ch, 0, vict, TO_VICT);
    }
  }
}



ACMD(do_insult)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *victim;

  one_argument(argument, arg);

  if (*arg) {
    if (!(victim = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM)))
      send_to_char(ch, "Can't hear you!\r\n");
    else {
      if (victim != ch) {
	send_to_char(ch, "You insult %s.\r\n", GET_NAME(victim));

	switch (rand_number(0, 2)) {
	case 0:
	  if (GET_SEX(ch) == SEX_MALE) {
	    if (GET_SEX(victim) == SEX_MALE)
	      act("$n accuses you of fighting like a woman!", FALSE, ch, 0, victim, TO_VICT);
	    else
	      act("$n says that women can't fight.", FALSE, ch, 0, victim, TO_VICT);
	  } else {		/* Ch == Woman */
	    if (GET_SEX(victim) == SEX_MALE)
	      act("$n accuses you of having the smallest... (brain?)",
		  FALSE, ch, 0, victim, TO_VICT);
	    else
	      act("$n tells you that you'd lose a beauty contest against a troll.",
		  FALSE, ch, 0, victim, TO_VICT);
	  }
	  break;
	case 1:
	  act("$n calls your mother a bitch!", FALSE, ch, 0, victim, TO_VICT);
	  break;
	default:
	  act("$n tells you to get lost!", FALSE, ch, 0, victim, TO_VICT);
	  break;
	}			/* end switch */

	act("$n insults $N.", TRUE, ch, 0, victim, TO_NOTVICT);
      } else {			/* ch == victim */
	send_to_char(ch, "You feel insulted.\r\n");
      }
    }
  } else
    send_to_char(ch, "I'm sure you don't want to insult *everybody*...\r\n");
}


char *fread_action(FILE *fl, int nr)
{
  char buf[MAX_STRING_LENGTH];

  fgets(buf, MAX_STRING_LENGTH, fl);
  if (feof(fl)) {
    log("SYSERR: fread_action: unexpected EOF near action #%d", nr);
    /*  SYSERR_DESC:
     *  fread_action() will fail if it discovers an end of file marker
     *  before it is able to read in the expected string.  This can be
     *  caused by a truncated socials file.
     */
    exit(1);
  }
  if (*buf == '#')
    return (NULL);

  buf[strlen(buf) - 1] = '\0';
  return (strdup(buf));
}


void free_social_messages(void)
{
  int ac;
  struct social_messg *soc;

  for (ac = 0; ac <= list_top; ac++) {
    soc = &soc_mess_list[ac];

    if (soc->char_no_arg)	free(soc->char_no_arg);
    if (soc->others_no_arg)	free(soc->others_no_arg);
    if (soc->char_found)	free(soc->char_found);
    if (soc->others_found)	free(soc->others_found);
    if (soc->vict_found)	free(soc->vict_found);
    if (soc->not_found)		free(soc->not_found);
    if (soc->char_auto)		free(soc->char_auto);
    if (soc->others_auto)	free(soc->others_auto);
  }
  free(soc_mess_list);
}


void boot_social_messages(void)
{
  FILE *fl;
  int nr, i, hide, min_pos, curr_soc = -1;
  char next_soc[100];
  struct social_messg temp;

  /* open social file */
  if (!(fl = fopen(SOCMESS_FILE, "r"))) {
    log("SYSERR: can't open socials file '%s': %s", SOCMESS_FILE, strerror(errno));
    /*  SYSERR_DESC:
     *  This error, from boot_social_messages(), occurs when the server
     *  fails to open the file containing the social messages.  The error
     *  at the end will indicate the reason why.
     */
    exit(1);
  }
  /* count socials & allocate space */
  for (nr = 0; *cmd_info[nr].command != '\n'; nr++)
    if (cmd_info[nr].command_pointer == do_action)
      list_top++;

  CREATE(soc_mess_list, struct social_messg, list_top + 1);

  /* now read 'em */
  for (;;) {
    fscanf(fl, " %s ", next_soc);
    if (*next_soc == '$')
      break;
    if (fscanf(fl, " %d %d \n", &hide, &min_pos) != 2) {
      log("SYSERR: format error in social file near social '%s'", next_soc);
      /*  SYSERR_DESC:
       *  From boot_social_messages(), this error is output when the
       *  server is expecting to find the remainder of the first line of the
       *  social ('hide' and 'minimum position').  These must follow the
       *  name of the social with a single space such as: 'accuse 0 5\n'.
       *  This error often occurs when one of the numbers is missing or the
       *  social name has a space in it (i.e., 'bend over').
       */
      exit(1);
    }
    if (++curr_soc > list_top) {
      log("SYSERR: Ran out of slots in social array. (%d > %d)", curr_soc, list_top);
      /*  SYSERR_DESC:
       *  The server creates enough space for all of the socials that it finds
       *  in the command structure (cmd_info[] in interpreter.c).  These are
       *  designated with the 'do_action' command call.  If there are more
       *  socials in the file than in the cmd_info structure, the
       *  boot_social_messages() function will fail with this error.
       */
      break;
    }
 
    /* read the stuff */
    soc_mess_list[curr_soc].act_nr = nr = find_command(next_soc);
    soc_mess_list[curr_soc].hide = hide;
    soc_mess_list[curr_soc].min_victim_position = min_pos;

#ifdef CIRCLE_ACORN
    if (fgetc(fl) != '\n')
      log("SYSERR: Acorn bug workaround failed.");
      /*  SYSERR_DESC:
       *  The only time that this error should ever arise is if you are running
       *  your CircleMUD on the Acorn platform.  The error arises when the
       *  server cannot properly read a '\n' out of the file at the end of the
       *  first line of the social (that with 'hide' and 'min position').  This
       *  is in boot_social_messages().
       */
#endif

    soc_mess_list[curr_soc].char_no_arg = fread_action(fl, nr);
    soc_mess_list[curr_soc].others_no_arg = fread_action(fl, nr);
    soc_mess_list[curr_soc].char_found = fread_action(fl, nr);

    /* if no char_found, the rest is to be ignored */
    if (!soc_mess_list[curr_soc].char_found)
      continue;

    soc_mess_list[curr_soc].others_found = fread_action(fl, nr);
    soc_mess_list[curr_soc].vict_found = fread_action(fl, nr);
    soc_mess_list[curr_soc].not_found = fread_action(fl, nr);
    soc_mess_list[curr_soc].char_auto = fread_action(fl, nr);
    soc_mess_list[curr_soc].others_auto = fread_action(fl, nr);

    /* If social not found, re-use this slot.  'curr_soc' will be reincremented. */
    if (nr < 0) {
      log("SYSERR: Unknown social '%s' in social file.", next_soc);
      /*  SYSERR_DESC:
       *  This occurs when the find_command() function in interpreter.c cannot
       *  find the social of the name in the file in the cmd_info[] structure.
       *  This is returned to boot_social_messages(), and the function
       *  reassigns that slot to another social to avoid running out of memory.
       *  The solution is to add this social to the cmd_info[] array in
       *  interpreter.c or to remove the social from the file.
       */
      memset(&soc_mess_list[curr_soc--], 0, sizeof(struct social_messg));
      continue;
    }

    /* If the command we found isn't do_action, we didn't count it for the CREATE(). */
    if (cmd_info[nr].command_pointer != do_action) {
      log("SYSERR: Social '%s' already assigned to a command.", next_soc);
      /*  SYSERR_DESC:
       *  This error occurs when boot_social_messages() reads in a social and
       *  then discovers that it is not assigned 'do_action' as a command in
       *  the cmd_info[] array in interpreter.c
       */
      memset(&soc_mess_list[curr_soc--], 0, sizeof(struct social_messg));
    }
  }

  /* close file & set top */
  fclose(fl);
  list_top = curr_soc;

  /* now, sort 'em */
  for (curr_soc = 0; curr_soc < list_top; curr_soc++) {
    min_pos = curr_soc;
    for (i = curr_soc + 1; i <= list_top; i++)
      if (soc_mess_list[i].act_nr < soc_mess_list[min_pos].act_nr)
	min_pos = i;
    if (curr_soc != min_pos) {
      temp = soc_mess_list[curr_soc];
      soc_mess_list[curr_soc] = soc_mess_list[min_pos];
      soc_mess_list[min_pos] = temp;
    }
  }
}
