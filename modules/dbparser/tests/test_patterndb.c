#include "apphook.h"
#include "tags.h"
#include "logmsg.h"
#include "messages.h"
#include "filter.h"
#include "patterndb.h"
#include "plugin.h"

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <glib/gstdio.h>

gboolean fail = FALSE;
gboolean verbose = FALSE;

#define test_fail(fmt, args...) \
do {\
  printf(fmt, ##args); \
  fail = TRUE; \
} while (0);

#define test_msg(fmt, args...) \
do { \
  if (verbose) printf(fmt, ##args); \
} while (0);

#define MYHOST "MYHOST"


void
create_pattern_db(PatternDB **patterndb, gchar *pdb, gchar ** filename)
{
  *patterndb = pattern_db_new();

  g_file_open_tmp("patterndbXXXXXX.xml", filename, NULL);
  g_file_set_contents(*filename, pdb, strlen(pdb), NULL);

  if (pattern_db_load(*patterndb, configuration, *filename, NULL))
    {
      if (!g_str_equal((*patterndb)->version, "3"))
        test_fail("Invalid version '%s'\n", (*patterndb)->version);
      if (!g_str_equal((*patterndb)->pub_date, "2010-02-22"))
        test_fail("Invalid pub_date '%s'\n", (*patterndb)->pub_date);
    }
  else
    {
      fail = TRUE;
    }
}

void
clean_pattern_db(PatternDB **patterndb, gchar **filename)
{
  pattern_db_free(*patterndb);
  *patterndb = NULL;

  g_unlink(*filename);
  g_free(*filename);

}

/* pdb skeleton used to test patterndb rule actions. E.g. whenever a rule
 * matches, certain actions described in the rule need to be performed.
 * This tests those */
gchar *pdb_ruletest_skeleton = "<patterndb version='3' pub_date='2010-02-22'>\
 <ruleset name='testset' id='1'>\
  <patterns>\
    <pattern>prog1</pattern>\
    <pattern>prog2</pattern>\
  </patterns>\
  <rule provider='test' id='11' class='system' context-scope='program' context-id='$PID' context-timeout='60'>\
   <patterns>\
    <pattern>pattern11</pattern>\
    <pattern>pattern11a</pattern>\
   </patterns>\
   <tags>\
    <tag>tag11-1</tag>\
    <tag>tag11-2</tag>\
   </tags>\
   <values>\
    <value name='n11-1'>v11-1</value>\
    <value name='n11-2'>v11-2</value>\
    <value name='vvv'>${HOST}</value>\
   </values>\
   <actions>\
     <action rate='1/60' condition='\"${n11-1}\" == v11-1' trigger='match'>\
       <message>\
         <value name='MESSAGE'>rule11 matched</value>\
         <tags>\
           <tag>tag11-3</tag>\
         </tags>\
       </message>\
     </action>\
     <action rate='1/60' condition='\"${n11-1}\" == v11-1' trigger='timeout'>\
       <message>\
         <value name='MESSAGE'>rule11 timed out</value>\
         <tags>\
           <tag>tag11-4</tag>\
         </tags>\
       </message>\
     </action>\
   </actions>\
  </rule>\
  <rule provider='test' id='12' class='violation'>\
   <patterns>\
    <pattern>pattern12</pattern>\
    <pattern>pattern12a</pattern>\
   </patterns>\
  </rule>\
 </ruleset>\
</patterndb>";


void
test_rule_value(PatternDB *patterndb, const gchar *pattern, const gchar *name, const gchar *value)
{
  gboolean result;
  LogMessage *msg = log_msg_new_empty();
  gboolean found = FALSE;
  const gchar *val;
  gssize len;

  log_msg_set_value(msg, LM_V_MESSAGE, pattern, strlen(pattern));
  log_msg_set_value(msg, LM_V_PROGRAM, "prog1", 5);
  log_msg_set_value(msg, LM_V_HOST, MYHOST, strlen(MYHOST));

  result = pattern_db_process(patterndb, msg, NULL);
  val = log_msg_get_value(msg, log_msg_get_value_handle(name), &len);

  if (!!value ^ (len > 0))
    test_fail("Value '%s' is %smatching for pattern '%s' (%d)\n", name, found ? "" : "not ", pattern, !!result);

  log_msg_unref(msg);
  pattern_db_forget_state(patterndb);
}

void
test_rule_tag(PatternDB *patterndb, const gchar *pattern, const gchar *tag, gboolean set)
{
  LogMessage *msg = log_msg_new_empty();
  gboolean found, result;

  log_msg_set_value(msg, LM_V_MESSAGE, pattern, strlen(pattern));
  log_msg_set_value(msg, LM_V_PROGRAM, "prog2", 5);

  result = pattern_db_process(patterndb, msg, NULL);
  found = log_msg_is_tag_by_name(msg, tag);

  if (set ^ found)
    test_fail("Tag '%s' is %sset for pattern '%s' (%d)\n", tag, found ? "" : "not ", pattern, !!result);

  log_msg_unref(msg);
  pattern_db_forget_state(patterndb);
}

void
test_rule_action_message_value(PatternDB *patterndb, const gchar *pattern, gint trigger, const gchar *value, const gchar *expected)
{
  LogMessage *msg = log_msg_new_empty();
  gboolean found, result;

  log_msg_set_value(msg, LM_V_MESSAGE, pattern, strlen(pattern));
  log_msg_set_value(msg, LM_V_PROGRAM, "prog2", 5);

  result = pattern_db_process(patterndb, msg, NULL);
#if 0
  val = log_msg_get_value(msg, log_msg_get_value_handle(name), &len);

  if (!!value ^ (len > 0))
    test_fail("Value '%s' is %smatching for pattern '%s' (%d)\n", name, found ? "" : "not ", pattern, !!result);
#endif
  log_msg_unref(msg);
  pattern_db_forget_state(patterndb);
}

void
test_rule_action_message_tag(PatternDB *patterndb, const gchar *pattern, gint trigger, const gchar *tag, gboolean set)
{
  LogMessage *msg = log_msg_new_empty();
  gboolean found, result;

  log_msg_set_value(msg, LM_V_MESSAGE, pattern, strlen(pattern));
  log_msg_set_value(msg, LM_V_PROGRAM, "prog2", 5);

  result = pattern_db_process(patterndb, msg, NULL);
#if 0
  found = log_msg_is_tag_by_name(msg, tag);

  if (set ^ found)
    test_fail("Tag '%s' is %sset for pattern '%s' (%d)\n", tag, found ? "" : "not ", pattern, !!result);
#endif
  log_msg_unref(msg);
  pattern_db_forget_state(patterndb);
}


void
test_patterndb_rule(void)
{
  PatternDB *patterndb;
  gchar *filename = NULL;

  create_pattern_db(&patterndb, pdb_ruletest_skeleton, &filename);
  test_rule_tag(patterndb, "pattern11", "tag11-1", TRUE);
  test_rule_tag(patterndb, "pattern11", ".classifier.system", TRUE);
  test_rule_tag(patterndb, "pattern11", "tag11-2", TRUE);
  test_rule_tag(patterndb, "pattern11", "tag11-3", FALSE);
  test_rule_tag(patterndb, "pattern11a", "tag11-1", TRUE);
  test_rule_tag(patterndb, "pattern11a", "tag11-2", TRUE);
  test_rule_tag(patterndb, "pattern11a", "tag11-3", FALSE);
  test_rule_tag(patterndb, "pattern12", ".classifier.violation", TRUE);
  test_rule_tag(patterndb, "pattern12", "tag12-1", FALSE);
  test_rule_tag(patterndb, "pattern12", "tag12-2", FALSE);
  test_rule_tag(patterndb, "pattern12", "tag12-3", FALSE);
  test_rule_tag(patterndb, "pattern12a", "tag12-1", FALSE);
  test_rule_tag(patterndb, "pattern12a", "tag12-2", FALSE);
  test_rule_tag(patterndb, "pattern12a", "tag12-3", FALSE);
  test_rule_tag(patterndb, "pattern1x", "tag1x-1", FALSE);
  test_rule_tag(patterndb, "pattern1x", "tag1x-2", FALSE);
  test_rule_tag(patterndb, "pattern1x", "tag1x-3", FALSE);
  test_rule_tag(patterndb, "pattern1xa", "tag1x-1", FALSE);
  test_rule_tag(patterndb, "pattern1xa", "tag1x-2", FALSE);
  test_rule_tag(patterndb, "pattern1xa", "tag1x-3", FALSE);

  test_rule_value(patterndb, "pattern11", "n11-1", "v11-1");
  test_rule_value(patterndb, "pattern11", ".classifier.class", "system");
  test_rule_value(patterndb, "pattern11", "n11-2", "v11-2");
  test_rule_value(patterndb, "pattern11", "n11-3", NULL);
  test_rule_value(patterndb, "pattern11a", "n11-1", "v11-1");
  test_rule_value(patterndb, "pattern11a", "n11-2", "v11-2");
  test_rule_value(patterndb, "pattern11a", "n11-3", NULL);
  test_rule_value(patterndb, "pattern12", ".classifier.class", "violation");
  test_rule_value(patterndb, "pattern12", "n12-1", NULL);
  test_rule_value(patterndb, "pattern12", "n12-2", NULL);
  test_rule_value(patterndb, "pattern12", "n12-3", NULL);
  test_rule_value(patterndb, "pattern1x", "n1x-1", NULL);
  test_rule_value(patterndb, "pattern1x", "n1x-2", NULL);
  test_rule_value(patterndb, "pattern1x", "n1x-3", NULL);
  test_rule_value(patterndb, "pattern11", "vvv", MYHOST);

  test_rule_action_message_value(patterndb, "pattern11", RAT_MATCH, "MESSAGE", "rule11 matched");
  test_rule_action_message_tag(patterndb, "pattern11", RAT_MATCH, "tag11-3", TRUE);
  test_rule_action_message_tag(patterndb, "pattern11", RAT_MATCH, "tag11-4", FALSE);

  clean_pattern_db(&patterndb, &filename);
}

gchar *pdb_parser_skeleton_prefix ="<?xml version='1.0' encoding='UTF-8'?>\
          <patterndb version='3' pub_date='2010-02-22'>\
            <ruleset name='test1_program' id='480de478-d4a6-4a7f-bea4-0c0245d361e1'>\
                <pattern>test</pattern>\
                    <rule id='1' class='test1' provider='my'>\
                        <patterns>\
                         <pattern>";

gchar *pdb_parser_skeleton_postfix =  "</pattern>\
                      </patterns>\
                    </rule>\
            </ruleset>\
        </patterndb>";


void
test_pattern(PatternDB *patterndb, const gchar *pattern, const gchar *rule, gboolean match)
{
  gboolean result;
  LogMessage *msg = log_msg_new_empty();
  static LogTemplate *templ;

  GString *res = g_string_sized_new(128);
  static TimeZoneInfo *tzinfo = NULL;

  if (!tzinfo)
    tzinfo = time_zone_info_new(NULL);
  if (!templ)
    templ = log_template_new(configuration, "dummy", "$TEST");

  log_msg_set_value(msg, LM_V_HOST, MYHOST, strlen(MYHOST));
  log_msg_set_value(msg, LM_V_PROGRAM, "test", strlen(MYHOST));
  log_msg_set_value(msg, LM_V_MESSAGE, pattern, strlen(pattern));

  result = pattern_db_process(patterndb, msg, NULL);

  log_template_format(templ, msg, NULL, LTZ_LOCAL, 0, res);

  if (strcmp(res->str, pattern) == 0)
    {
       test_msg("Rule: '%s' Value '%s' is inserted into $TEST res:(%s)\n", rule, pattern, res->str);
    }

  if ((match && !result) || (!match && result))
     {
       test_fail("FAIL: Value '%s' is %smatching for pattern '%s' \n", rule, !!result ? "" : "not ", pattern);
     }
   else
    {
       test_msg("Value '%s' is %smatching for pattern '%s' \n", rule, !!result ? "" : "not ", pattern);
     }

  g_string_free(res, TRUE);

  log_msg_unref(msg);
}

void
test_parser(gchar **test)
{
  PatternDB *patterndb;
  GString *str;
  gchar *filename = NULL;
  gint index = 1;

  str = g_string_new(pdb_parser_skeleton_prefix);
  g_string_append(str, test[0]);
  g_string_append(str, pdb_parser_skeleton_postfix);

  create_pattern_db(&patterndb, str->str, &filename);
  while(test[index] != NULL)
    test_pattern(patterndb, test[index++], test[0], TRUE);
  while(test[index] != NULL)
    test_pattern(patterndb, test[index++], test[0], FALSE);

  clean_pattern_db(&patterndb, &filename);
}

gchar * test1 [] = {
"@ANYSTRING:TEST@",
"ab ba ab",
"ab ba ab",
"1234ab",
"ab1234",
"1.2.3.4",
"ab  1234  ba",
"&lt;ab ba&gt;",
NULL,NULL
};

gchar * test2 [] = {
"@DOUBLE:TEST@",
"1234",
"1234.567",
"1.2.3.4",
"1234ab",
NULL, // not match
"ab1234",NULL
};

gchar * test3 [] = {
"@ESTRING:TEST:endmark@",
"ab ba endmark",
NULL,
"ab ba",NULL
};

gchar * test4 [] = {
"@ESTRING:TEST:&gt;@",
"ab ba > ab",
NULL,
"ab ba",NULL
};

gchar * test5 [] = {
"@FLOAT:TEST@",
"1234",
"1234.567",
"1.2.3.4",
"1234ab",
NULL, // not match
"ab1234",NULL
};

gchar * test6 [] = {
"@IPv4:TEST@",
"1.2.3.4",
"0.0.0.0",
"255.255.255.255",
NULL,
"256.256.256.256",
"1.2.3.4.5",
"1234",
"ab1234",
"ab1.2.3.4",
"1,2,3,4",NULL
};

gchar * test7 [] = {
"@IPv6:TEST@",
"2001:0db8:0000:0000:0000:0000:1428:57ab",
"2001:0db8:0000:0000:0000::1428:57ab",
"2001:0db8:0:0:0:0:1428:57ab",
"2001:0db8:0:0::1428:57ab",
"2001:0db8::1428:57ab",
"2001:db8::1428:57ab",
NULL,
"2001:0db8::34d2::1428:57ab",NULL
};

gchar * test8 [] = {
"@IPvANY:TEST@",
"1.2.3.4",
"0.0.0.0",
"255.255.255.255",
"2001:0db8:0000:0000:0000:0000:1428:57ab",
"2001:0db8:0000:0000:0000::1428:57ab",
"2001:0db8:0:0:0:0:1428:57ab",
"2001:0db8:0:0::1428:57ab",
"2001:0db8::1428:57ab",
"2001:db8::1428:57ab",
NULL,
"256.256.256.256",
"1.2.3.4.5",
"1234",
"ab1234",
"ab1.2.3.4",
"1,2,3,4",
"2001:0db8::34d2::1428:57ab",NULL
};

gchar * test9 [] = {
"@NUMBER:TEST@",
"1234",
"1.2",
"1.2.3.4",
"1234ab",
NULL,
"ab1234",
"1,2",NULL
};

gchar * test10 [] = {
"@QSTRING:TEST:&lt;&gt;@",
"<aa bb>",
"< aabb >",
NULL,
"aabb>",
"<aabb",NULL
};

gchar * test11 [] = {
"@STRING:TEST@",
"aabb",
"aa bb",
"1234",
"ab1234",
"1234bb",
"1.2.3.4",
NULL,
"aa bb",NULL
};

gchar **parsers[] = {test1, test2, test3, test4, test5, test6, test7, test8, test9, test10, test11, NULL};

void
test_patterndb_parsers()
{
  gint i;

  for (i = 0; parsers[i]; i++)
    {
      test_parser(parsers[i]);
    }
}

int
main(int argc, char *argv[])
{

  app_startup();

  if (argc > 1)
    verbose = TRUE;

  msg_init(TRUE);

  configuration = cfg_new(0x0302);
  plugin_load_module("syslogformat", configuration, NULL);

  pattern_db_global_init();

  test_patterndb_rule();
  test_patterndb_parsers();

  app_shutdown();
  return  (fail ? 1 : 0);
}