#include <stdio.h>
#include <gcc-plugin.h>
#include "plugin-version.h"
#include <context.h>
#include <tree.h>
#include <tree-pass.h>
#include <rtl.h>
#include <memmodel.h>
#include <emit-rtl.h>

#define TODO_dump_func 0

#define P(...) {printf("[+] " __VA_ARGS__); printf("\n");}

// We must assert that this plugin is GPL compatible
int plugin_is_GPL_compatible;

static void setup_others(rtx_insn * insn);
static void finish_others(rtx_insn * insn);
static void setup_main(rtx_insn * insn);
static void finish_main(rtx_insn * insn);

// Dispatcher for epilogue and prologue modifier functions
typedef void(* rad)(rtx_insn * insn);
struct {rad setup; rad finish;} rads[]=
{
    {setup_main,  finish_main},
    {setup_others, finish_others},
};                

static void setup_main(rtx_insn * insn){
	// Call cfi_pa_init from the shared library
    
	P("In main() prologue");

    rtx call = gen_rtx_SYMBOL_REF(Pmode, "cfi_pa_init");
    // Generate call instruction
    call = gen_rtx_CALL(Pmode, gen_rtx_MEM(FUNCTION_MODE, call), const0_rtx);
    // Emit call instruction
    rtx_insn *last = emit_insn_before(call, insn);

	// Call cfi_pa_encrypt from the shared library
    rtx s_call = gen_rtx_SYMBOL_REF(Pmode, "cfi_pa_encrypt");
    s_call = gen_rtx_CALL(Pmode, gen_rtx_MEM(FUNCTION_MODE, s_call), const0_rtx);
    emit_insn_after(s_call, last);

    //print_rtl_single(stderr, set);
}


static void finish_main(rtx_insn * insn){

	// Call cfi_pa_decrypt from the shared library
    rtx call = gen_rtx_SYMBOL_REF(Pmode, "cfi_pa_decrypt");
    call = gen_rtx_CALL(Pmode, gen_rtx_MEM(FUNCTION_MODE, call), const0_rtx);
    rtx_insn *last = emit_insn_after(call, insn);

	// Call cfi_pa_exit from the shared library
    rtx s_call = gen_rtx_SYMBOL_REF(Pmode, "cfi_pa_exit");
    s_call = gen_rtx_CALL(Pmode, gen_rtx_MEM(FUNCTION_MODE, s_call), const0_rtx);
    emit_insn_after(s_call, last);
}


static void setup_others(rtx_insn * insn){

	// Call cfi_pa_encrypt from the shared library
    rtx call = gen_rtx_SYMBOL_REF(Pmode, "cfi_pa_encrypt");
    call = gen_rtx_CALL(Pmode, gen_rtx_MEM(FUNCTION_MODE, call), const0_rtx);
    emit_insn_before(call, insn);
}


static void finish_others(rtx_insn * insn){
	
	// Call cfi_pa_decrypt from the shared library
    rtx call = gen_rtx_SYMBOL_REF(Pmode, "cfi_pa_decrypt");
    call = gen_rtx_CALL(Pmode, gen_rtx_MEM(FUNCTION_MODE, call), const0_rtx);
    //rtx_insn *last = emit_insn_after(call, insn);
    emit_insn_after(call, insn);
}


static bool cfi_gate(void){
    return true;
}


static unsigned cfi_enforce(void){
	rtx_insn * insn;
    const char * name = get_name(cfun->decl);
    P("Dealing with: %s", name);

    // loop through instructions
    for (insn=get_insns(); insn; insn=NEXT_INSN(insn)){
	   
	   	// check if we are in the prologue
    	if (NOTE_P(insn) && (NOTE_KIND(insn) == NOTE_INSN_PROLOGUE_END))
        {
            if (name[0] == 'm' && name[1] == 'a' && name[2] == 'i' && name[3] == 'n'){
                /* in the prologue of main() */
                printf("main prologue here\n");
                rads[0].setup(insn);
            }
            else{
                /* in the prologue of other functions */
                printf("others prologue here\n");
                rads[1].setup(insn);
            }
        }

		// check if we are in the epilogue
        else if (NOTE_P(insn) && (NOTE_KIND(insn) == NOTE_INSN_EPILOGUE_BEG))
        {   
            if (name[0] == 'm' && name[1] == 'a' && name[2] == 'i' && name[3] == 'n'){
                /* in the epilogue of main() */
                rads[0].finish(insn);
                printf("main epilogue here\n"); 
            }
            else{
                /* in the epilogue of other functions */
                printf("others epilogue here\n"); 
                rads[1].finish(insn);
            }
        }
    }
	return 0;
}

/* old structure used for C compability (GCC transition from C to C++)
   .type is the field where we need to specify RT_PASS to work at RTL level
*/
const pass_data my_pass_data = {
    .type = RTL_PASS,					 /* type */
    .name = "cfi_banzai", 				 /* name */
    .optinfo_flags = OPTGROUP_NONE,      /* optinfo_flags */
    .tv_id = TV_NONE,					 /* tv_id */
    .properties_required = 0,            /* properties_required */
    .properties_provided = 0,            /* properties_provided */
    .properties_destroyed = 0,			 /* properties_destroyed */
    .todo_flags_start = 0,               /* todo_flags_start */
    .todo_flags_finish = TODO_dump_func, /* todo_flags_finish */  
};


/*  pass is represented as a class that inherits from a GCC class called opt_pass
    This plugin works at RTL level so it will inherit from rtl_opt_pass
*/
struct my_pass : rtl_opt_pass{
public:
    my_pass(gcc::context *ctx) : rtl_opt_pass(my_pass_data,ctx) {}

	/* The virtual method 'execute' of a pass will be called by GCC for each function being compiled */
    virtual unsigned int execute(function* fun) override
    {
        printf("%s\n", function_name(fun));
		
		/* call cfi_enforce*/
        cfi_enforce();
        return 0;
    }

    virtual bool gate(function* fun) override
    {
        return cfi_gate();
        
    }

    virtual my_pass* clone() override {return this; }
};


// new pass that will be registered
static struct register_pass_info my_pass_info = {
	// insert my_pass into the struct used to register the pass
    .pass = new my_pass(g),
	 // get called after pro_and_epilogue pass (RTL passes)
    .reference_pass_name = "pro_and_epilogue",
    .ref_pass_instance_number = 0,
    .pos_op = PASS_POS_INSERT_AFTER,
};


/**
 * Initializes the plugin. Returns 0 if initialization finishes successfully. 
 */
int plugin_init(struct plugin_name_args *plugin_info, struct plugin_gcc_version *version){
	// this plugin is compatible only with specified base ver
    if (!plugin_default_version_check (version, &gcc_version)){
        printf("GCC incompatible version\n");
        return 1;
    }
	// tell to GCC some info about this plugin
    register_callback("cfi_banzai", PLUGIN_PASS_MANAGER_SETUP, NULL, &my_pass_info);
    return 0;
}
