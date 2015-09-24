#include <stdint.h>

// MuValue and MuXxxValue type are opaque handles to values in the Mu type
// system.
//
// The actual values are held by MuCtx. MuValue opaquely refers to one such
// value. Copies of MuValue values refer to the same value. A MuValue instance
// can only be used in the MuCtx holding it.
//
// Values of subtypes can be cast to MuValue and back using the type cast
// expression in C, similar to casting one pointer to another.
typedef void *MuValue;
typedef void *MuIntValue;
typedef void *MuFloatValue;
typedef void *MuDoubleValue;
typedef void *MuRefValue;
typedef void *MuIRefValue;
typedef void *MuStructValue;
typedef void *MuFuncRefValue;
typedef void *MuThreadRefValue;
typedef void *MuStackRefValue;
typedef void *MuTagRef64Value;
typedef void *MuUPtrValue;
typedef void *MuUFPValue;

// Identifiers and names of Mu
typedef uint32_t MuID;
typedef char *MuName;

// Convenient types for the void* type and the void(*)() type in C
typedef void *MuCPtr;
typedef void (*MuCFP)();

// Result of a trap handler
typedef int MuTrapHandlerResult;

#define MU_THREAD_EXIT          0x00
#define MU_REBIND_PASS_VALUE    0x01
#define MU_REBIND_PASS_VOID     0x02
#define MU_REBIND_THROW_EXC     0x03

// Declare the types here because they are used in the following signatures.
typedef struct MuVM MuVM;
typedef struct MuCtx MuCtx;

// Signature of the trap handler
typedef void (*MuTrapHandler)(MuCtx *ctx, MuThreadRefValue thread,
        MuStackRefValue stack, int wpid, MuTrapHandlerResult *result,
        MuStackRefValue *new_stack, MuValue *value, MuRefValue *exception);

// Signature of the undefined funciton handler
typedef void (*MuUndefFuncHandler)(MuCtx *ctx, MuID func_id);

// Memory orders
typedef int MuMemOrd;

#define MU_NOT_ATOMIC  0x00
#define MU_RELAXED     0x01
#define MU_CONSUME     0x02
#define MU_ACQUIRE     0x03
#define MU_RELEASE     0x04
#define MU_ACQ_REL     0x05
#define MU_SEQ_CST     0x06

// Operations for the atomicrmw API function
typedef int MuAtomicRMWOp;

#define MU_XCHG        0x00
#define MU_ADD         0x01
#define MU_SUB         0x02
#define MU_AND         0x03
#define MU_NAND        0x04
#define MU_OR          0x05
#define MU_XOR         0x06
#define MU_MAX         0x07
#define MU_MIN         0x08
#define MU_UMAX        0x09
#define MU_UMIN        0x0A

// Calling conventions.
typedef int MuCallConv;

#define MU_DEFUALT     0x00
// Concrete Mu implementations may define more calling conventions.

// NOTE: MuVM and MuCtx are structures with many function pointers. This
// approach loosens the coupling between the client module and the Mu
// implementation.  At compile time, the client does not need to link against
// any dynamic libraries. At run time, more than one Mu implementations can be
// used by the same client.

// A handle and method lists of a micro VM
struct MuVM {
    void *header;   // Refer to internal stuff

    // Create context
    MuCtx*  (*new_context)(MuVM *mvm);
    
    // Convert between IDs and names
    MuID    (*id_of  )(MuVM *mvm, MuName name);
    MuName  (*name_of)(MuVM *mvm, MuID id);

    // Set handlers
    void    (*set_trap_handler      )(MuVM *mvm, MuTrapHandler trap_handler);
    void    (*set_undef_func_handler)(MuVM *mvm, MuUndefFuncHandler undef_func_handler);
};

// A local context. It can only be used by one thread at a time. It holds many
// states which are typically held by a Mu thread, such as object references,
// local heap allocation pool, and an object-pinning set. It also holds many Mu
// values and expose them to the client as opaque handles (MuValue and its
// subtypes).
struct MuCtx {
    void *header;   // Refer to internal stuff

    // Convert between IDs and names
    MuID        (*id_of  )(MuCtx *ctx, MuName name);
    MuName      (*name_of)(MuCtx *ctx, MuID id);

    // Close the current context, releasing all resources
    void        (*close_context)(MuCtx *ctx);

    // Load bundles and HAIL scripts
    void        (*load_bundle)(MuCtx *ctx, char *buf, int sz);
    void        (*load_hail  )(MuCtx *ctx, char *buf, int sz);

    // Convert from C values to Mu values
    MuIntValue      (*handle_from_int8  )(MuCtx *ctx, int8_t   num, int len);
    MuIntValue      (*handle_from_uint8 )(MuCtx *ctx, uint8_t  num, int len);
    MuIntValue      (*handle_from_int16 )(MuCtx *ctx, int16_t  num, int len);
    MuIntValue      (*handle_from_uint16)(MuCtx *ctx, uint16_t num, int len);
    MuIntValue      (*handle_from_int32 )(MuCtx *ctx, int32_t  num, int len);
    MuIntValue      (*handle_from_uint32)(MuCtx *ctx, uint32_t num, int len);
    MuIntValue      (*handle_from_int64 )(MuCtx *ctx, int64_t  num, int len);
    MuIntValue      (*handle_from_uint64)(MuCtx *ctx, uint64_t num, int len);
    MuFloatValue    (*handle_from_float )(MuCtx *ctx, float    num);
    MuDoubleValue   (*handle_from_double)(MuCtx *ctx, double   num);
    MuUPtrValue     (*handle_from_ptr   )(MuCtx *ctx, MuID mu_type, MuCPtr ptr);
    MuUFPValue      (*handle_from_fp    )(MuCtx *ctx, MuID mu_type, MuCFP fp);

    // Convert from Mu values to C values
    int8_t      (*handle_to_sint8 )(MuCtx *ctx, MuIntValue    opnd);
    uint8_t     (*handle_to_uint8 )(MuCtx *ctx, MuIntValue    opnd);
    int16_t     (*handle_to_sint16)(MuCtx *ctx, MuIntValue    opnd);
    uint16_t    (*handle_to_uint16)(MuCtx *ctx, MuIntValue    opnd);
    int32_t     (*handle_to_sint32)(MuCtx *ctx, MuIntValue    opnd);
    uint32_t    (*handle_to_uint32)(MuCtx *ctx, MuIntValue    opnd);
    int64_t     (*handle_to_sint64)(MuCtx *ctx, MuIntValue    opnd);
    uint64_t    (*handle_to_uint64)(MuCtx *ctx, MuIntValue    opnd);
    float       (*handle_to_float )(MuCtx *ctx, MuFloatValue  opnd);
    double      (*handle_to_double)(MuCtx *ctx, MuDoubleValue opnd);
    MuCPtr      (*handle_to_ptr   )(MuCtx *ctx, MuUPtrValue   opnd);
    MuCFP       (*handle_to_fp    )(MuCtx *ctx, MuUFPValue    opnd);

    // Make MuValue instances from Mu global SSA variables
    MuValue         (*handle_from_const )(MuCtx *ctx, MuID id);
    MuIRefValue     (*handle_from_global)(MuCtx *ctx, MuID id);
    MuFuncRefValue  (*handle_from_func  )(MuCtx *ctx, MuID id);
    MuValue         (*handle_from_expose)(MuCtx *ctx, MuID id);

    // Delete the value held by the MuCtx, making it unusable, but freeing up
    // the resource.
    void        (*delete_value)(MuCtx *ctx, MuValue opnd);

    // Manipulate Mu values of the struct<...> type
    MuValue     (*extract_value)(MuCtx *ctx, MuStructValue str, int index);
    MuValue     (*insert_value )(MuCtx *ctx, MuStructValue str, int index, MuValue newval);

    // Heap allocation
    MuRefValue  (*new_fixed )(MuCtx *ctx, MuID mu_type);
    MuRefValue  (*new_hybrid)(MuCtx *ctx, MuID mu_type, MuIntValue length);

    // Change the T or sig in ref<T>, iref<T> or func<sig>
    MuValue     (*refcast)(MuCtx *ctx, MuValue opnd, MuID new_type);

    // Memory addressing
    MuIRefValue     (*get_iref           )(MuCtx *ctx, MuRefValue opnd);
    MuIRefValue     (*get_field_iref     )(MuCtx *ctx, MuIRefValue opnd, int field);
    MuIRefValue     (*get_elem_iref      )(MuCtx *ctx, MuIRefValue opnd, MuIntValue index);
    MuIRefValue     (*shift_iref         )(MuCtx *ctx, MuIRefValue opnd, MuIntValue offset);
    MuIRefValue     (*get_fixed_part_iref)(MuCtx *ctx, MuIRefValue opnd);
    MuIRefValue     (*get_var_part_iref  )(MuCtx *ctx, MuIRefValue opnd);

    // Memory accessing
    MuValue     (*load     )(MuCtx *ctx, MuMemOrd ord, MuIRefValue loc);
    void        (*store    )(MuCtx *ctx, MuMemOrd ord, MuIRefValue loc, MuValue newval);
    MuValue     (*cmpxchg  )(MuCtx *ctx, MuMemOrd ord_succ, MuMemOrd ord_fail,
                        int weak, MuIRefValue loc, MuValue expected, MuValue desired,
                        int *is_succ);
    MuValue     (*atomicrmw)(MuCtx *ctx, MuMemOrd ord, MuAtomicRMWOp op,
                        MuIRefValue loc, MuValue opnd);
    void        (*fence    )(MuCtx *ctx, MuMemOrd ord);

    // Thread and stack creation and stack destruction
    MuStackRefValue     (*new_stack )(MuCtx *ctx, MuFuncRefValue func, MuValue *args, int nargs);
    MuThreadRefValue    (*new_thread)(MuCtx *ctx, MuStackRefValue stack);
    void                (*kill_stack)(MuCtx *ctx, MuStackRefValue stack);

    // Stack introspection
    MuID        (*cur_func_ver   )(MuCtx *ctx, MuStackRefValue stack, int frame);
    MuID        (*cur_inst       )(MuCtx *ctx, MuStackRefValue stack, int frame);
    void        (*dump_keepalives)(MuCtx *ctx, MuStackRefValue stack, int frame, MuValue *results);
    
    // On-stack replacement
    void        (*pop_frame )(MuCtx *ctx, MuStackRefValue stack);
    void        (*push_frame)(MuCtx *ctx, MuStackRefValue stack, MuFuncRefValue func, MuValue *args, int nargs);

    // 64-bit tagged reference operations
    int             (*tr64_is_fp   )(MuCtx *ctx, MuTagRef64Value value);
    int             (*tr64_is_int  )(MuCtx *ctx, MuTagRef64Value value);
    int             (*tr64_is_ref  )(MuCtx *ctx, MuTagRef64Value value);
    MuDoubleValue   (*tr64_to_fp   )(MuCtx *ctx, MuTagRef64Value value);
    MuIntValue      (*tr64_to_int  )(MuCtx *ctx, MuTagRef64Value value);
    MuRefValue      (*tr64_to_ref  )(MuCtx *ctx, MuTagRef64Value value);
    MuIntValue      (*tr64_to_tag  )(MuCtx *ctx, MuTagRef64Value value);
    MuTagRef64Value (*tr64_from_fp )(MuCtx *ctx, MuDoubleValue value);
    MuTagRef64Value (*tr64_from_int)(MuCtx *ctx, MuIntValue value);
    MuTagRef64Value (*tr64_from_ref)(MuCtx *ctx, MuRefValue ref, MuIntValue tag);

    // Watchpoint operations
    void        (*enable_watchpoint )(MuCtx *ctx, int wpid);
    void        (*disable_watchpoint)(MuCtx *ctx, int wpid);

    // Mu memory pinning, usually object pinning
    MuUPtrValue (*pin  )(MuCtx *ctx, MuValue loc);      // loc is either MuRefValue or MuIRefValue
    void        (*unpin)(MuCtx *ctx, MuValue loc);      // loc is either MuRefValue or MuIRefValue

    // Expose Mu functions as native callable things, usually function pointers
    MuValue     (*expose  )(MuCtx *ctx, MuFuncRefValue func, MuCallConv call_conv, MuIntValue cookie);
    void        (*unexpose)(MuCtx *ctx, MuValue value);
};

