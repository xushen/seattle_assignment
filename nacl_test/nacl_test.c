#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ppapi/c/pp_errors.h"
#include "ppapi/c/pp_module.h"
#include "ppapi/c/pp_var.h"
#include "ppapi/c/ppb.h"
#include "ppapi/c/ppb_instance.h"
#include "ppapi/c/ppb_messaging.h"
#include "ppapi/c/ppb_var.h"
#include "ppapi/c/ppp.h"
#include "ppapi/c/ppp_instance.h"
#include "ppapi/c/ppp_messaging.h"

static PPB_Messaging* ppb_messaging_interface = NULL;
static PPB_Var* ppb_var_interface = NULL;

/**
 * Because different string type of javascript and c/c++.
 * Communication need to transfer c string from javascript var
 * @var_string[PP_Var] string from browser
 * @return c string.
 */
static char* TransCStrFromVar(struct PP_Var var_string)
{
  char *cstr = NULL;
  const char* var_cstr = NULL;
  uint32_t length = 0;

  if (ppb_var_interface == NULL){
    return NULL;
  }

  
  var_cstr= ppb_var_interface->VarToUtf8(var_string, &length);
  if (length <= 0){
    return NULL;
  }

  cstr = (char *)malloc(length + 1);
  memcpy(cstr, var_cstr, length);
  cstr[length] = '\0';
  return cstr;  
}

/**
 * Because different string type of javascript and c/c++.
 * Communication need to transfer javascript var from c 
 * @var_string[PP_Var] string from native client
 * @return javascript string.
 */
static struct PP_Var TransVarFromCStr(const char* str)
{
  if (ppb_var_interface != NULL){
    return ppb_var_interface->VarFromUtf8(str, strlen(str));
  }

  return PP_MakeUndefined();
}

/**
 * Handle messages from browser.
 * Note: when initializing PPP, need hook this function as a callback function 
 */
void Messaging_HandleMessage(PP_Instance instance, struct PP_Var var_message)
{
  struct PP_Var var_str = PP_MakeUndefined();
  char* c_str = NULL;

  if (var_message.type != PP_VARTYPE_STRING)
  {
    return;
  }

  c_str = TransCStrFromVar(var_message);
  if (c_str == NULL){
    return;
  }
  
  var_str = TransVarFromCStr(c_str);
  ppb_messaging_interface->PostMessage(instance, var_str);

  return;
}

static PP_Bool Instance_DidCreate(PP_Instance instance,
                                  uint32_t argc,
                                  const char* argn[],
                                  const char* argv[]) {

  return PP_TRUE;
}


static void Instance_DidDestroy(PP_Instance instance) {
}

static void Instance_DidChangeView(PP_Instance instance,
                                   PP_Resource view_resource) {
}

static void Instance_DidChangeFocus(PP_Instance instance,
                                    PP_Bool has_focus) {
}

static PP_Bool Instance_HandleDocumentLoad(PP_Instance instance,
                                           PP_Resource url_loader) {
  return PP_FALSE;
}

PP_EXPORT int32_t PPP_InitializeModule(PP_Module a_module_id,
                                       PPB_GetInterface get_browser) {
  ppb_messaging_interface =
      (PPB_Messaging*)(get_browser(PPB_MESSAGING_INTERFACE));
  ppb_var_interface = (PPB_Var*)(get_browser(PPB_VAR_INTERFACE));
  return PP_OK;
}

PP_EXPORT const void* PPP_GetInterface(const char* interface_name) {
  if (strcmp(interface_name, PPP_INSTANCE_INTERFACE) == 0) {
    static PPP_Instance instance_interface = {
      &Instance_DidCreate,
      &Instance_DidDestroy,
      &Instance_DidChangeView,
      &Instance_DidChangeFocus,
      &Instance_HandleDocumentLoad,
    };
    return &instance_interface;
  } else if (strcmp(interface_name, PPP_MESSAGING_INTERFACE) == 0){
    static PPP_Messaging message_interface = {
      &Messaging_HandleMessage
    };
    return &message_interface;
  }

  return NULL;
}

PP_EXPORT void PPP_ShutdownModule() {
}
