#include <stdint.h>

typedef intptr_t MuHandle;
typedef uint32_t MuID;
typedef char *MuName;
typedef void *MuPtr;
typedef void (*MuFP)();

typedef int MuTrapHandlerResult;

#define MU_THREAD_EXIT          0x00
#define MU_REBIND_PASS_VALUE    0x01
#define MU_REBIND_PASS_VOID     0x02
#define MU_REBIND_THROW_EXC     0x03

typedef void (*MuTrapHandler)(MuCtx *ctx, MuHandle thread, MuHandle stack, int wpid,
                            MuTrapHandlerResult *result, MuHandle *newstack,

typedef void (*MuUndefFuncHandler)(MuCtx *ctx, MuID func_id);

typedef int MuMemOrd;

#define MU_NOT_ATOMIC  0x00
#define MU_RELAXED     0x01
#define MU_CONSUME     0x02
#define MU_ACQUIRE     0x03
#define MU_RELEASE     0x04
#define MU_ACQ_REL     0x05
#define MU_SEQ_CST     0x06

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

typedef struct MuVM MuVM;
typedef struct MuVM {
    void *header;
    MuCtx*  (*new_context)(MuVM *mvm);
    MuID    (*id_of)(MuVM *mvm, MuName name);
    MuName  (*name_of)(MuVM *mvm, MuID name);
    void    (*set_trap_handler)(MuVM *mvm, MuTrapHandler trap_handler);
    void    (*set_undef_func_handler)(MuVM *mvm, MuUndefFuncHandler undef_func_handler);
};

typedef struct MuCtx MuCtx;
typedef struct MuCtx {
    void *header;
    MuID        (*id_of)(MuCtx *ctx, MuName name);
    MuName      (*name_of)(MuCtx *ctx, MuID name);
    void        (*close_context)(MuCtx *ctx);
    MuHandle    (*load_bundle)(MuCtx *ctx, char *buf, int sz);
    MuHandle    (*load_hail  )(MuCtx *ctx, char *buf, int sz);

    MuHandle    (*handle_from_int64 )(MuCtx *ctx, int64_t  num, int len);
    MuHandle    (*handle_from_uint64)(MuCtx *ctx, uint64_t num, int len);
    MuHandle    (*handle_from_float )(MuCtx *ctx, float    num);
    MuHandle    (*handle_from_double)(MuCtx *ctx, double   num);
    MuHandle    (*handle_from_ptr   )(MuCtx *ctx, MuID mu_type, MuPtr ptr);
    MuHandle    (*handle_from_fp    )(MuCtx *ctx, MuID mu_type, MuFP fp);

    MuHandle    (*handle_from_const )(MuCtx *ctx, MuID id);
    MuHandle    (*handle_from_global)(MuCtx *ctx, MuID id);
    MuHandle    (*handle_from_func  )(MuCtx *ctx, MuID id);
    MuHandle    (*handle_from_expose)(MuCtx *ctx, MuID id);

    int64_t     (*handle_to_sint64)(MuCtx *ctx, MuHandle opnd);
    uint64_t    (*handle_to_uint64)(MuCtx *ctx, MuHandle opnd);
    float       (*handle_to_float )(MuCtx *ctx, MuHandle opnd);
    double      (*handle_to_double)(MuCtx *ctx, MuHandle opnd);
    MuPtr       (*handle_to_ptr   )(MuCtx *ctx, MuHandle opnd);
    MuFP        (*handle_to_fp    )(MuCtx *ctx, MuHandle opnd);

    void        (*delete_handle)(MuCtx *ctx, MuHandle opnd);

    MuHandle    (*extract_value)(MuCtx *ctx, MuHandle str, int index);
    MuHandle    (*insert_value )(MuCtx *ctx, MuHandle str, int index, MuHandle newval);

    MuHandle    (*new_fixed )(MuCtx *ctx, MuID mu_type);
    MuHandle    (*new_hybrid)(MuCtx *ctx, MuID mu_type, uint64_t length);

    MuHandle    (*refcast)(MuCtx *ctx, MuHandle opnd, MuID new_type);

    MuHandle    (*get_iref)(MuCtx *ctx, MuHandle opnd);
    MuHandle    (*get_field_iref     )(MuCtx *ctx, MuHandle opnd, int field);
    MuHandle    (*get_elem_iref      )(MuCtx *ctx, MuHandle opnd, int64_t index);
    MuHandle    (*shift_iref         )(MuCtx *ctx, MuHandle opnd, int64_t offset);
    MuHandle    (*get_fixed_part_iref)(MuCtx *ctx, MuHandle opnd);
    MuHandle    (*get_var_part_iref  )(MuCtx *ctx, MuHandle opnd);

    MuHandle    (*load     )(MuCtx *ctx, MuMemOrd ord, MuHandle loc);
    void        (*store    )(MuCtx *ctx, MuMemOrd ord, MuHandle loc, MuHandle newval);
    MuHandle    (*cmpxchg  )(MuCtx *ctx, MuMemOrd ord_succ, MuMemOrd ord_fail,
                        int weak, MuHandle loc, MuHandle expected, MuHandle desired,
                        int *is_succ);
    MuHandle    (*atomicrmw)(MuCtx *ctx, MuMemOrd ord, MuAtomicRMWOp op,
                        MuHandle loc, MuHandle, opnd);
    void        (*fence    )(MuCtx *ctx, MuMemOrd ord);

    MuHandle    (*new_stack  )(MuCtx *ctx, MuID func, MuHandle *args, int nargs);
    MuHandle    (*new_thread )(MuCtx *ctx, MuHandle stack);
    void        (*kill_thread)(MuCtx *ctx, MuHandle stack);

    MuID        (*cur_func_ver   )(MuCtx *ctx, MuHandle stack, int frame);
    MuID        (*cur_inst       )(MuCtx *ctx, MuHandle stack, int frame);
    void        (*dump_keepalives)(MuCtx *ctx, MuHandle stack, int frame, MuHandle *results);
    
    void        (*pop_frame )(MuCtx *ctx, MuHandle stack);
    void        (*push_frame)(MuCtx *ctx, MuHandle stack, MuID func, MuHandle *args, int nargs);

    int         (*tr64_is_fp   )(MuCtx *ctx, MuHandle value);
    int         (*tr64_is_int  )(MuCtx *ctx, MuHandle value);
    int         (*tr64_is_ref  )(MuCtx *ctx, MuHandle value);
    double      (*tr64_to_fp   )(MuCtx *ctx, MuHandle value);
    uint64_t    (*tr64_to_int  )(MuCtx *ctx, MuHandle value);
    MuHandle    (*tr64_to_ref  )(MuCtx *ctx, MuHandle value);
    uint64_t    (*tr64_to_tag  )(MuCtx *ctx, MuHandle value);
    MuHandle    (*tr64_from_fp )(MuCtx *ctx, double value);
    MuHandle    (*tr64_from_int)(MuCtx *ctx, uint64_t value);
    MuHandle    (*tr64_from_ref)(MuCtx *ctx, MuHandle ref, uint64_t tag);

    void        (*enable_watchpoint )(MuCtx *ctx, int wpid);
    void        (*disable_watchpoint)(MuCtx *ctx, int wpid);

    MuPtr       (*pin  )(MuCtx *ctx, MuHandle ref);
    void        (*unpin)(MuCtx *ctx, MuHandle ref);

    MuFP        (*expose  )(MuCtx *ctx, MuHandle func, MuCallConv call_conv, uint64_t cookie);
    void        (*unexpose)(MuCtx *ctx, MuFP value);
};

