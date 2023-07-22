#include <gcc-plugin.h>
#include <stdio.h>
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

// Push a register on the stack
static inline rtx push_reg(unsigned int reg) {
    rtx r, dec, mem, psh;
    r = gen_rtx_REG(DImode, reg);
    dec = gen_rtx_PRE_DEC(DImode, stack_pointer_rtx);
    mem = gen_rtx_MEM(DImode, dec);
    psh = gen_rtx_SET(mem, r);
    return psh;
}

// Pop a register from the stack
static inline rtx pop_reg(unsigned int reg) {
    rtx r, inc, mem, pop;
    r = gen_rtx_REG(DImode, reg);
    inc = gen_rtx_POST_INC(DImode, stack_pointer_rtx);
    mem = gen_rtx_MEM(DImode, inc);
    pop = gen_rtx_SET(r, mem);
    return pop;
}

/*
* In the x86_64 calling convention the first six arguments of a function call
* are passed in the rdi, rsi, rdx, rcx, r8, r9 registers, so since cfi_pa_init 
* and cfi_pa_encrypt may modify the contents of those registers, their content 
* must be pushed on the stack before calling cfi_pa_init()/cfi_pa_encrypt() and 
* popped when cfi_pa_encrypt() returns 
*/
// Save registers containing function call arguments
static rtx_insn* save_regs_before(rtx_insn *insn) {
    rtx_insn *last = emit_insn_before(push_reg(DI_REG), insn);
    last = emit_insn_after(push_reg(SI_REG), last);
    last = emit_insn_after(push_reg(DX_REG), last);
    last = emit_insn_after(push_reg(CX_REG), last);
    last = emit_insn_after(push_reg(R8_REG), last);
    last = emit_insn_after(push_reg(R9_REG), last);
    last = emit_insn_after(push_reg(AX_REG), last);

    return last;
}

// Restore registers containing function call arguments
static rtx_insn* restore_regs_after(rtx_insn *insn) {
    rtx_insn *last = emit_insn_after(pop_reg(AX_REG), insn);
    last = emit_insn_after(pop_reg(R9_REG), last);
    last = emit_insn_after(pop_reg(R8_REG), last);
    last = emit_insn_after(pop_reg(CX_REG), last);
    last = emit_insn_after(pop_reg(DX_REG), last);
    last = emit_insn_after(pop_reg(SI_REG), last);
    last = emit_insn_after(pop_reg(DI_REG), last);

    return last;
}

static void setup_main(rtx_insn * insn){
    P("In main() prologue");

    // rtx call = gen_rtx_SYMBOL_REF(Pmode, "cfi_pa_init");
    // Generate call instruction
    // call = gen_rtx_CALL(Pmode, gen_rtx_MEM(FUNCTION_MODE, call), const0_rtx);
    // Emit call instruction
    // rtx_insn *init_call = emit_insn_before(call, insn);
    
    rtx s_call = gen_rtx_SYMBOL_REF(Pmode, "cfi_pa_encrypt");
    s_call = gen_rtx_CALL(Pmode, gen_rtx_MEM(FUNCTION_MODE, s_call), const0_rtx);
    rtx_insn *last_call = emit_insn_before(s_call, insn);
    save_regs_before(last_call);
    restore_regs_after(last_call);
    //print_rtl_single(stderr, set);
}


static void finish_main(rtx_insn * insn){

	// Call cfi_pa_decrypt from the shared library
    rtx call = gen_rtx_SYMBOL_REF(Pmode, "cfi_pa_decrypt");
    call = gen_rtx_CALL(Pmode, gen_rtx_MEM(FUNCTION_MODE, call), const0_rtx);
    rtx_insn *last = emit_insn_after(call, insn);
    emit_insn_before(push_reg(AX_REG), last);
    emit_insn_after(pop_reg(AX_REG), last);
}


static void setup_others(rtx_insn * insn){

	// Call cfi_pa_encrypt from the shared library
    rtx call = gen_rtx_SYMBOL_REF(Pmode, "cfi_pa_encrypt");
    call = gen_rtx_CALL(Pmode, gen_rtx_MEM(FUNCTION_MODE, call), const0_rtx);
    rtx_insn *last = emit_insn_before(call, insn);
    save_regs_before(last);
    restore_regs_after(last);
}


static void finish_others(rtx_insn * insn){
	
	// Call cfi_pa_decrypt from the shared library
    rtx call = gen_rtx_SYMBOL_REF(Pmode, "cfi_pa_decrypt");
    call = gen_rtx_CALL(Pmode, gen_rtx_MEM(FUNCTION_MODE, call), const0_rtx);
    rtx_insn *last = emit_insn_after(call, insn);
    emit_insn_before(push_reg(AX_REG), last);
    emit_insn_after(pop_reg(AX_REG), last);
    //save_regs_before(last);
    //restore_regs_after(last);
}


static bool cfi_gate(void){
    return true;
}


static unsigned cfi_enforce(void){
	rtx_insn * insn;
    const char * name = get_name(cfun->decl);
    //P("Dealing with: %s", name);

    // loop through instructions
    for (insn=get_insns(); insn; insn=NEXT_INSN(insn)) {
	   	// check if we are in the prologue
    	if (NOTE_P(insn) && (NOTE_KIND(insn) == NOTE_INSN_PROLOGUE_END))
        {
            if (name[0] == 'm' && name[1] == 'a' && name[2] == 'i' && name[3] == 'n'){
                /* in the prologue of main() */
                //printf("main prologue here\n");
                rads[0].setup(insn);
            }
            else{
                /* in the prologue of other functions */
                //printf("others prologue here\n");
                rads[1].setup(insn);
            }
        }

		// check if we are in the epilogue
        else if (NOTE_P(insn) && (NOTE_KIND(insn) == NOTE_INSN_EPILOGUE_BEG))
        {   
            if (name[0] == 'm' && name[1] == 'a' && name[2] == 'i' && name[3] == 'n'){
                /* in the epilogue of main() */
                rads[0].finish(insn);
                //printf("main epilogue here\n"); 
            }
            else{
                /* in the epilogue of other functions */
                //printf("others epilogue here\n"); 
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
        //printf("%s\n", function_name(fun));
		
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
