#ifndef __MUAPI_H__
#define __MUAPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// ABOUT HEADER PARSERS
//
// This header is designed to be processed by a regular-expression-based parser
// so that it will be convenient to to generate language bindings. There is one
// such parser in the reference implementation v2: muapiparser.py
//
// It can identify the type hierarchies of the typedefs below, so language
// bindings can auto-generate a class hierarchy for MuValue handles if the
// high-level language supports, such as Python. For the convenience of this
// purpose, all typedefs are in the simplest form: "typedef FromTy ToType;"
// where FromTy may end with an asterisk '*'. Typedefs of function pointer types
// are deliberately separated into two steps (see MuCFP below), because
// regexp-based parsers may not be smart enough to parse function pointer types.
//
// It can find all definitions of macro constants. These constants, such as
// MU_BINOP_ADD, all have prefixes to make parsing easy.
//
// It can find methods of MuVM and MuCtx which are structs of function pointers.
// Parameter types use high-level typedef-ed types so that the language binding
// generator has more information (such as MuInstResNode) than the raw C types
// (such as void*).
//
// The "/// MUAPIPARSER" comments provide extra annotations (pragmas) for the
// method just before it. Multiple pragmas are separated by semicolons ';'.
//
//  param:array:sz_param
//      means param is a pointer to an array, and the effective length is
//      determined by the sz_param parameter. param can be NULL, in which case
//      it is considered as a 0-length array.
//  param:optional
//      means param is a pointer and it may be NULL.
//  param:out
//      means param is a pointer and it is supposed to be used as an output
//      parameter. Values should be written in rather than read out of it.


// MuValue and MuXxxValue type are opaque handles to values in the Mu type
// system.
//
// The actual values are held by MuCtx. MuValue opaquely refers to one such
// value. Copies of MuValue values refer to the same value. A MuValue instance
// can only be used in the MuCtx holding it.
//
// Values of subtypes can be cast to/from their abstract parents using the type
// cast expression in C, similar to casting one pointer to another.

// Top value type.
typedef void *MuValue;                // Any Mu value

// Abstract value type.
typedef MuValue MuSeqValue;           // array or vector
typedef MuValue MuGenRefValue;        // ref, iref, funcref, threadref, stackref, framecursorref, irnoderef

// concrete value types
typedef MuValue MuIntValue;           // int<n>
typedef MuValue MuFloatValue;         // float
typedef MuValue MuDoubleValue;        // double
typedef MuValue MuUPtrValue;          // uptr
typedef MuValue MuUFPValue;           // ufuncptr

typedef MuSeqValue MuStructValue;     // struct<...>
typedef MuSeqValue MuArrayValue;      // array<T l>
typedef MuSeqValue MuVectorValue;     // vector<T l>

typedef MuGenRefValue MuRefValue;           // ref<T>
typedef MuGenRefValue MuIRefValue;          // iref<T>
typedef MuGenRefValue MuTagRef64Value;      // tagref64
typedef MuGenRefValue MuFuncRefValue;       // funcref<sig>
typedef MuGenRefValue MuThreadRefValue;     // threadref
typedef MuGenRefValue MuStackRefValue;      // stackref
typedef MuGenRefValue MuFCRefValue;         // framecursorref
typedef MuGenRefValue MuIRNodeRefValue;     // irnoderef

// Subtypes of MuIRNodeRefValue. These are used in the IR Builder API.

// Shorter aliases
typedef MuIRNodeRefValue MuIRNode;      // All IR Nodes

// IR node reference hierarchy
typedef MuIRNode MuBundleNode;          // Bundle

// NOTE: All MuChildNode can have ID (ctx->get_id) and name (ctx->set_name)
typedef MuIRNode MuChildNode;           // All children of bundle
typedef MuChildNode MuTypeNode;             // Type
typedef MuChildNode MuFuncSigNode;          // Function signature
typedef MuChildNode MuVarNode;              // Variable
typedef MuVarNode MuGlobalVarNode;          // Global variable
typedef MuGlobalVarNode MuConstNode;            // Constant
typedef MuGlobalVarNode MuGlobalNode;           // Global cell
typedef MuGlobalVarNode MuFuncNode;             // Function
typedef MuGlobalVarNode MuExpFuncNode;          // Exposed function
typedef MuVarNode MuLocalVarNode;           // Local variable
typedef MuLocalVarNode MuNorParamNode;          // Normal parameter
typedef MuLocalVarNode MuExcParamNode;          // Exception parameter
typedef MuLocalVarNode MuInstResNode;           // Instruction result
typedef MuChildNode MuFuncVerNode;          // Function version
typedef MuChildNode MuBBNode;               // Basic block
typedef MuChildNode MuInstNode;             // Instruction (itself, not result)

// Identifiers and names of Mu
typedef uint32_t MuID;
typedef char *MuName;

// Convenient types for the void* type and the void(*)() type in C
typedef void *MuCPtr;
typedef void _MuCFP_Func();
typedef _MuCFP_Func* MuCFP;

// Boolean type. The size of the C99 standard bool type seems to vary a lot
// among ABIs. So we use int instead. In C, the result type of relational and
// logical expressions are int.
typedef int MuBool;

// Use uintptr_t for the size of all array parameters.
typedef uintptr_t MuArraySize;

// Watch point ID
typedef uint32_t MuWPID;

// Super type for numerical flags used by Mu.
typedef uint32_t MuFlag;

// Result of a trap handler
typedef MuFlag MuTrapHandlerResult;

// Values or MuTrapHandlerResult
#define MU_THREAD_EXIT          ((MuTrapHandlerResult)0x00)
#define MU_REBIND_PASS_VALUES   ((MuTrapHandlerResult)0x01)
#define MU_REBIND_THROW_EXC     ((MuTrapHandlerResult)0x02)

// Used by MuTrapHandler
typedef void _MuValuesFreer_Func(MuValue *values, MuCPtr freerdata);
typedef _MuValuesFreer_Func* MuValuesFreer;

// Declare the types here because they are used in the following signatures.
typedef struct MuVM MuVM;
typedef struct MuCtx MuCtx;

// Signature of the trap handler
typedef void _MuTrapHandler_Func(
        // input parameters
        MuCtx *ctx,
        MuThreadRefValue thread,
        MuStackRefValue stack,
        MuWPID wpid,
        // output parameters
        MuTrapHandlerResult *result,
        MuStackRefValue *new_stack,
        MuValue **values,
        MuArraySize *nvalues,
        MuValuesFreer *freer,
        MuCPtr *freerdata,
        MuRefValue *exception,
        // input parameter (userdata)
        MuCPtr userdata);
typedef _MuTrapHandler_Func* MuTrapHandler;

// Destination kinds
typedef MuFlag MuDestKind;
#define MU_DEST_NORMAL      ((MuDeskKind)0x01)
#define MU_DEST_EXCEPT      ((MuDeskKind)0x02)
#define MU_DEST_TRUE        ((MuDeskKind)0x03)
#define MU_DEST_FALSE       ((MuDeskKind)0x04)
#define MU_DEST_DEFAULT     ((MuDeskKind)0x05)
#define MU_DEST_DISABLED    ((MuDeskKind)0x06)
#define MU_DEST_ENABLED     ((MuDeskKind)0x07)

// Binary operators
typedef MuFlag MuBinOptr;
#define MU_BINOP_ADD    ((MuBinOptr)0x01)
#define MU_BINOP_SUB    ((MuBinOptr)0x02)
#define MU_BINOP_MUL    ((MuBinOptr)0x03)
#define MU_BINOP_SDIV   ((MuBinOptr)0x04)
#define MU_BINOP_SREM   ((MuBinOptr)0x05)
#define MU_BINOP_UDIV   ((MuBinOptr)0x06)
#define MU_BINOP_UREM   ((MuBinOptr)0x07)
#define MU_BINOP_SHL    ((MuBinOptr)0x08)
#define MU_BINOP_LSHR   ((MuBinOptr)0x09)
#define MU_BINOP_ASHR   ((MuBinOptr)0x0A)
#define MU_BINOP_AND    ((MuBinOptr)0x0B)
#define MU_BINOP_OR     ((MuBinOptr)0x0C)
#define MU_BINOP_XOR    ((MuBinOptr)0x0D)
#define MU_BINOP_FADD   ((MuBinOptr)0xB0)
#define MU_BINOP_FSUB   ((MuBinOptr)0xB1)
#define MU_BINOP_FMUL   ((MuBinOptr)0xB2)
#define MU_BINOP_FDIV   ((MuBinOptr)0xB3)
#define MU_BINOP_FREM   ((MuBinOptr)0xB4)

// Comparing operators
typedef MuFlag MuCmpOptr;
#define MU_CMP_EQ       ((MuCmpOptr)0x20)
#define MU_CMP_NE       ((MuCmpOptr)0x21)
#define MU_CMP_SGE      ((MuCmpOptr)0x22)
#define MU_CMP_SGT      ((MuCmpOptr)0x23)
#define MU_CMP_SLE      ((MuCmpOptr)0x24)
#define MU_CMP_SLT      ((MuCmpOptr)0x25)
#define MU_CMP_UGE      ((MuCmpOptr)0x26)
#define MU_CMP_UGT      ((MuCmpOptr)0x27)
#define MU_CMP_ULE      ((MuCmpOptr)0x28)
#define MU_CMP_ULT      ((MuCmpOptr)0x29)
#define MU_CMP_FFALSE   ((MuCmpOptr)0xC0)
#define MU_CMP_FTRUE    ((MuCmpOptr)0xC1)
#define MU_CMP_FUNO     ((MuCmpOptr)0xC2)
#define MU_CMP_FUEQ     ((MuCmpOptr)0xC3)
#define MU_CMP_FUNE     ((MuCmpOptr)0xC4)
#define MU_CMP_FUGT     ((MuCmpOptr)0xC5)
#define MU_CMP_FUGE     ((MuCmpOptr)0xC6)
#define MU_CMP_FULT     ((MuCmpOptr)0xC7)
#define MU_CMP_FULE     ((MuCmpOptr)0xC8)
#define MU_CMP_FORD     ((MuCmpOptr)0xC9)
#define MU_CMP_FOEQ     ((MuCmpOptr)0xCA)
#define MU_CMP_FONE     ((MuCmpOptr)0xCB)
#define MU_CMP_FOGT     ((MuCmpOptr)0xCC)
#define MU_CMP_FOGE     ((MuCmpOptr)0xCD)
#define MU_CMP_FOLT     ((MuCmpOptr)0xCE)
#define MU_CMP_FOLE     ((MuCmpOptr)0xCF)

// Conversion operators
typedef MuFlag MuConvOptr;
#define MU_CONV_TRUNC   ((MuConvOptr)0x30)
#define MU_CONV_ZEXT    ((MuConvOptr)0x31)
#define MU_CONV_SEXT    ((MuConvOptr)0x32)
#define MU_CONV_FPTRUNC ((MuConvOptr)0x33)
#define MU_CONV_FPEXT   ((MuConvOptr)0x34)
#define MU_CONV_FPTOUI  ((MuConvOptr)0x35)
#define MU_CONV_FPTOSI  ((MuConvOptr)0x36)
#define MU_CONV_UITOFP  ((MuConvOptr)0x37)
#define MU_CONV_SITOFP  ((MuConvOptr)0x38)
#define MU_CONV_BITCAST ((MuConvOptr)0x39)
#define MU_CONV_REFCAST ((MuConvOptr)0x3A)
#define MU_CONV_PTRCAST ((MuConvOptr)0x3B)

// Memory orders
typedef MuFlag MuMemOrd;
#define MU_ORD_NOT_ATOMIC   ((MuMemOrd)0x00)
#define MU_ORD_RELAXED      ((MuMemOrd)0x01)
#define MU_ORD_CONSUME      ((MuMemOrd)0x02)
#define MU_ORD_ACQUIRE      ((MuMemOrd)0x03)
#define MU_ORD_RELEASE      ((MuMemOrd)0x04)
#define MU_ORD_ACQ_REL      ((MuMemOrd)0x05)
#define MU_ORD_SEQ_CST      ((MuMemOrd)0x06)

// Operations for the atomicrmw API function
typedef MuFlag MuAtomicRMWOptr;
#define MU_ARMW_XCHG    ((MuAtomicRMWOptr)0x00)
#define MU_ARMW_ADD     ((MuAtomicRMWOptr)0x01)
#define MU_ARMW_SUB     ((MuAtomicRMWOptr)0x02)
#define MU_ARMW_AND     ((MuAtomicRMWOptr)0x03)
#define MU_ARMW_NAND    ((MuAtomicRMWOptr)0x04)
#define MU_ARMW_OR      ((MuAtomicRMWOptr)0x05)
#define MU_ARMW_XOR     ((MuAtomicRMWOptr)0x06)
#define MU_ARMW_MAX     ((MuAtomicRMWOptr)0x07)
#define MU_ARMW_MIN     ((MuAtomicRMWOptr)0x08)
#define MU_ARMW_UMAX    ((MuAtomicRMWOptr)0x09)
#define MU_ARMW_UMIN    ((MuAtomicRMWOptr)0x0A)

// Calling conventions.
typedef MuFlag MuCallConv;
#define MU_CC_DEFAULT   ((MuCallConv)0x00)
// Concrete Mu implementations may define more calling conventions.

// Common instructions.
typedef MuFlag MuCommInst;

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
    
    // Convert between IDs and names. Cannot be used on the bundles being built.
    MuID    (*id_of  )(MuVM *mvm, MuName name);
    MuName  (*name_of)(MuVM *mvm, MuID id);

    // Set handlers
    void    (*set_trap_handler      )(MuVM *mvm, MuTrapHandler trap_handler, MuCPtr userdata);
};

// A local context. It can only be used by one thread at a time. It holds many
// states which are typically held by a Mu thread, such as object references,
// local heap allocation pool, and an object-pinning set. It also holds many Mu
// values and expose them to the client as opaque handles (MuValue and its
// subtypes).
struct MuCtx {
    void *header;   // Refer to internal stuff

    // Convert between IDs and names. Cannot be used on the bundles being built.
    MuID        (*id_of  )(MuCtx *ctx, MuName name);
    MuName      (*name_of)(MuCtx *ctx, MuID id);

    // Close the current context, releasing all resources
    void        (*close_context)(MuCtx *ctx);

    // Load bundles and HAIL scripts
    void        (*load_bundle)(MuCtx *ctx, char *buf, MuArraySize sz); /// MUAPIPARSER buf:array:sz
    void        (*load_hail  )(MuCtx *ctx, char *buf, MuArraySize sz); /// MUAPIPARSER buf:array:sz

    // Convert from C values to Mu values
    MuIntValue      (*handle_from_sint8  )(MuCtx *ctx, int8_t     num, int len);
    MuIntValue      (*handle_from_uint8  )(MuCtx *ctx, uint8_t    num, int len);
    MuIntValue      (*handle_from_sint16 )(MuCtx *ctx, int16_t    num, int len);
    MuIntValue      (*handle_from_uint16 )(MuCtx *ctx, uint16_t   num, int len);
    MuIntValue      (*handle_from_sint32 )(MuCtx *ctx, int32_t    num, int len);
    MuIntValue      (*handle_from_uint32 )(MuCtx *ctx, uint32_t   num, int len);
    MuIntValue      (*handle_from_sint64 )(MuCtx *ctx, int64_t    num, int len);
    MuIntValue      (*handle_from_uint64 )(MuCtx *ctx, uint64_t   num, int len);
    MuIntValue      (*handle_from_uint64s)(MuCtx *ctx, uint64_t *nums, MuArraySize nnums, int len); /// MUAPIPARSER nums:array:nnums
    MuFloatValue    (*handle_from_float  )(MuCtx *ctx, float      num);
    MuDoubleValue   (*handle_from_double )(MuCtx *ctx, double     num);
    MuUPtrValue     (*handle_from_ptr    )(MuCtx *ctx, MuID mu_type, MuCPtr ptr);
    MuUFPValue      (*handle_from_fp     )(MuCtx *ctx, MuID mu_type, MuCFP fp);

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

    // Compare reference or general reference types.
    // EQ. Available for ref, iref, funcref, threadref and stackref.
    MuBool      (*ref_eq )(MuCtx *ctx, MuGenRefValue lhs, MuGenRefValue rhs);
    // ULT. Available for iref only.
    MuBool      (*ref_ult)(MuCtx *ctx, MuIRefValue lhs, MuIRefValue rhs);

    // Manipulate Mu values of the struct<...> type
    MuValue         (*extract_value)(MuCtx *ctx, MuStructValue str, int index);
    MuStructValue   (*insert_value )(MuCtx *ctx, MuStructValue str, int index, MuValue newval);

    // Manipulate Mu values of the array or vector type
    // str can be MuArrayValue or MuVectorValue
    MuValue     (*extract_element)(MuCtx *ctx, MuSeqValue str, MuIntValue index);
    MuSeqValue  (*insert_element )(MuCtx *ctx, MuSeqValue str, MuIntValue index, MuValue newval);

    // Heap allocation
    MuRefValue  (*new_fixed )(MuCtx *ctx, MuID mu_type);
    MuRefValue  (*new_hybrid)(MuCtx *ctx, MuID mu_type, MuIntValue length);

    // Change the T or sig in ref<T>, iref<T> or func<sig>
    MuGenRefValue   (*refcast)(MuCtx *ctx, MuGenRefValue opnd, MuID new_type);

    // Memory addressing
    MuIRefValue     (*get_iref           )(MuCtx *ctx, MuRefValue opnd);
    MuIRefValue     (*get_field_iref     )(MuCtx *ctx, MuIRefValue opnd, int field);
    MuIRefValue     (*get_elem_iref      )(MuCtx *ctx, MuIRefValue opnd, MuIntValue index);
    MuIRefValue     (*shift_iref         )(MuCtx *ctx, MuIRefValue opnd, MuIntValue offset);
    MuIRefValue     (*get_var_part_iref  )(MuCtx *ctx, MuIRefValue opnd);

    // Memory accessing
    MuValue     (*load     )(MuCtx *ctx, MuMemOrd ord, MuIRefValue loc);
    void        (*store    )(MuCtx *ctx, MuMemOrd ord, MuIRefValue loc, MuValue newval);
    MuValue     (*cmpxchg  )(MuCtx *ctx, MuMemOrd ord_succ, MuMemOrd ord_fail,
                        MuBool weak, MuIRefValue loc, MuValue expected, MuValue desired,
                        MuBool *is_succ); /// MUAPIPARSER is_succ:out
    MuValue     (*atomicrmw)(MuCtx *ctx, MuMemOrd ord, MuAtomicRMWOptr op,
                        MuIRefValue loc, MuValue opnd);
    void        (*fence    )(MuCtx *ctx, MuMemOrd ord);

    // Thread and stack creation and stack destruction
    MuStackRefValue     (*new_stack )(MuCtx *ctx, MuFuncRefValue func);
    MuThreadRefValue    (*new_thread_nor)(MuCtx *ctx, MuStackRefValue stack,
                            MuRefValue threadlocal,
                            MuValue *vals, MuBool nvals); /// MUAPIPARSER threadlocal:optional;vals:array:nvals
    MuThreadRefValue    (*new_thread_exc)(MuCtx *ctx, MuStackRefValue stack,
                            MuRefValue threadlocal,
                            MuRefValue exc); /// MUAPIPARSER threadlocal:optional
    void                (*kill_stack)(MuCtx *ctx, MuStackRefValue stack);

    // Thread-local object reference
    void        (*set_threadlocal)(MuCtx *ctx, MuThreadRefValue thread,
                    MuRefValue threadlocal);
    MuRefValue  (*get_threadlocal)(MuCtx *ctx, MuThreadRefValue thread);

    // Frame cursor operations
    MuFCRefValue    (*new_cursor  )(MuCtx *ctx, MuStackRefValue stack);
    void            (*next_frame  )(MuCtx *ctx, MuFCRefValue cursor);
    MuFCRefValue    (*copy_cursor )(MuCtx *ctx, MuFCRefValue cursor);
    void            (*close_cursor)(MuCtx *ctx, MuFCRefValue cursor);

    // Stack introspection
    MuID        (*cur_func       )(MuCtx *ctx, MuFCRefValue cursor);
    MuID        (*cur_func_ver   )(MuCtx *ctx, MuFCRefValue cursor);
    MuID        (*cur_inst       )(MuCtx *ctx, MuFCRefValue cursor);
    void        (*dump_keepalives)(MuCtx *ctx, MuFCRefValue cursor, MuValue *results); /// MUAPIPARSER results:out
    
    // On-stack replacement
    void        (*pop_frames_to)(MuCtx *ctx, MuFCRefValue cursor);
    void        (*push_frame   )(MuCtx *ctx, MuStackRefValue stack, MuFuncRefValue func);

    // 64-bit tagged reference operations
    MuBool          (*tr64_is_fp   )(MuCtx *ctx, MuTagRef64Value value);
    MuBool          (*tr64_is_int  )(MuCtx *ctx, MuTagRef64Value value);
    MuBool          (*tr64_is_ref  )(MuCtx *ctx, MuTagRef64Value value);
    MuDoubleValue   (*tr64_to_fp   )(MuCtx *ctx, MuTagRef64Value value);
    MuIntValue      (*tr64_to_int  )(MuCtx *ctx, MuTagRef64Value value);
    MuRefValue      (*tr64_to_ref  )(MuCtx *ctx, MuTagRef64Value value);
    MuIntValue      (*tr64_to_tag  )(MuCtx *ctx, MuTagRef64Value value);
    MuTagRef64Value (*tr64_from_fp )(MuCtx *ctx, MuDoubleValue value);
    MuTagRef64Value (*tr64_from_int)(MuCtx *ctx, MuIntValue value);
    MuTagRef64Value (*tr64_from_ref)(MuCtx *ctx, MuRefValue ref, MuIntValue tag);

    // Watchpoint operations
    void        (*enable_watchpoint )(MuCtx *ctx, MuWPID wpid);
    void        (*disable_watchpoint)(MuCtx *ctx, MuWPID wpid);

    // Mu memory pinning, usually object pinning
    MuUPtrValue (*pin  )(MuCtx *ctx, MuValue loc);      // loc is either MuRefValue or MuIRefValue
    void        (*unpin)(MuCtx *ctx, MuValue loc);      // loc is either MuRefValue or MuIRefValue

    // Expose Mu functions as native callable things, usually function pointers
    MuValue     (*expose  )(MuCtx *ctx, MuFuncRefValue func, MuCallConv call_conv, MuIntValue cookie);
    void        (*unexpose)(MuCtx *ctx, MuCallConv call_conv, MuValue value);

    /// IR Builder API from here on.

    // Create bundle
    MuBundleNode    (*new_bundle)(MuCtx *ctx);

    // Load a bundle built from an IR Builder into the micro VM
    void        (*load_bundle_from_node )(MuCtx *ctx, MuBundleNode b);
    // Call this function before the node is loaded to release all of its resources.
    void        (*abort_bundle_node     )(MuCtx *ctx, MuBundleNode b);

    // Get a MuChildNode that refers to an existing node that has the "id".
    // The returned handle is only usable within the bundle "b".
    // Can only get top-level definitions.
    MuChildNode (*get_node  )(MuCtx *ctx, MuBundleNode b, MuID id);

    // Get the ID of the IR node "node".
    MuID        (*get_id    )(MuCtx *ctx, MuBundleNode b, MuChildNode node);

    // Set the name of the IR node. MuName is '\0' terminated char*.
    void        (*set_name  )(MuCtx *ctx, MuBundleNode b, MuChildNode node, MuName name);

    /// Create top-level definitions. When created, they are added to the bundle "b".
    
    // Create types
    MuTypeNode  (*new_type_int      )(MuCtx *ctx, MuBundleNode b, int len);
    MuTypeNode  (*new_type_float    )(MuCtx *ctx, MuBundleNode b);
    MuTypeNode  (*new_type_double   )(MuCtx *ctx, MuBundleNode b);
    MuTypeNode  (*new_type_uptr     )(MuCtx *ctx, MuBundleNode b);
    void        (*set_type_uptr     )(MuCtx *ctx, MuTypeNode uptr, MuTypeNode ty);
    MuTypeNode  (*new_type_ufuncptr )(MuCtx *ctx, MuBundleNode b);
    void        (*set_type_ufuncptr )(MuCtx *ctx, MuTypeNode ufuncptr, MuFuncSigNode sig);

    MuTypeNode  (*new_type_struct   )(MuCtx *ctx, MuBundleNode b, MuTypeNode *fieldtys, MuArraySize nfieldtys);
                /// MUAPIPARSER fieldtys:array:nfieldtys
    MuTypeNode  (*new_type_hybrid   )(MuCtx *ctx, MuBundleNode b, MuTypeNode *fixedtys, MuArraySize nfixedtys, MuTypeNode varty);
                /// MUAPIPARSER fixedtys:array:nfixedtys
    MuTypeNode  (*new_type_array    )(MuCtx *ctx, MuBundleNode b, MuTypeNode elemty, uint64_t len);
    MuTypeNode  (*new_type_vector   )(MuCtx *ctx, MuBundleNode b, MuTypeNode elemty, uint64_t len);
    MuTypeNode  (*new_type_void     )(MuCtx *ctx, MuBundleNode b);

    MuTypeNode  (*new_type_ref      )(MuCtx *ctx, MuBundleNode b);
    void        (*set_type_ref      )(MuCtx *ctx, MuTypeNode ref, MuTypeNode ty);
    MuTypeNode  (*new_type_iref     )(MuCtx *ctx, MuBundleNode b);
    void        (*set_type_iref     )(MuCtx *ctx, MuTypeNode iref, MuTypeNode ty);
    MuTypeNode  (*new_type_weakref  )(MuCtx *ctx, MuBundleNode b);
    void        (*set_type_weakref  )(MuCtx *ctx, MuTypeNode weakref, MuTypeNode ty);
    MuTypeNode  (*new_type_funcref  )(MuCtx *ctx, MuBundleNode b);
    void        (*set_type_funcref  )(MuCtx *ctx, MuTypeNode funcref, MuFuncSigNode sig);
    MuTypeNode  (*new_type_tagref64 )(MuCtx *ctx, MuBundleNode b);

    MuTypeNode  (*new_type_threadref     )(MuCtx *ctx, MuBundleNode b);
    MuTypeNode  (*new_type_stackref      )(MuCtx *ctx, MuBundleNode b);
    MuTypeNode  (*new_type_framecursorref)(MuCtx *ctx, MuBundleNode b);
    MuTypeNode  (*new_type_irnoderef     )(MuCtx *ctx, MuBundleNode b);

    // Create function signatures
    MuFuncSigNode   (*new_funcsig   )(MuCtx *ctx, MuBundleNode b, MuTypeNode *paramtys, MuArraySize nparamtys, MuTypeNode *rettys, MuArraySize nrettys);
                    /// MUAPIPARSER paramtys:array:nparamtys;rettys:array:nrettys

    // Create constants
    // new_const_int works for int<n> with n <= 64, uptr and ufuncptr.
    MuConstNode (*new_const_int     )(MuCtx *ctx, MuBundleNode b, MuTypeNode ty, uint64_t value);
    // new_const_int_ex works for int<n> with n > 64. The number is segmented into 64-bit words, lower word first.
    MuConstNode (*new_const_int_ex  )(MuCtx *ctx, MuBundleNode b, MuTypeNode ty, uint64_t *values, MuArraySize nvalues); /// MUAPIPARSER values:array:nvalues
    // TODO: There is only one 'float' type and one 'double' type. Theoretically the 'ty' param is unnecessary
    // It is just added to mirror the text form. Eliminate them when we are ready to change the text form.
    MuConstNode (*new_const_float   )(MuCtx *ctx, MuBundleNode b, MuTypeNode ty, float value);
    MuConstNode (*new_const_double  )(MuCtx *ctx, MuBundleNode b, MuTypeNode ty, double value);
    // new_const_null works for all general reference types, but not uptr or ufuncptr.
    MuConstNode (*new_const_null    )(MuCtx *ctx, MuBundleNode b, MuTypeNode ty);
    // new_const_seq works for structs, arrays and vectors. Constants are non-recursive, so there is no set_const_seq.
    MuConstNode (*new_const_seq     )(MuCtx *ctx, MuBundleNode b, MuTypeNode ty, MuConstNode *elems, MuArraySize nelems); /// MUAPIPARSER elems:array:nelems
    
    // Create global cell
    MuGlobalNode (*new_global_cell  )(MuCtx *ctx, MuBundleNode b, MuTypeNode ty);

    // Create function
    MuFuncNode  (*new_func          )(MuCtx *ctx, MuBundleNode b, MuFuncSigNode sig);

    // Create function version
    // In order to declare and define a new function, the client must create
    // both a function node and a function version node.
    MuFuncVerNode   (*new_func_ver  )(MuCtx *ctx, MuBundleNode b, MuFuncNode func);

    // Create exposed function
    MuExpFuncNode   (*new_exp_func  )(MuCtx *ctx, MuBundleNode b, MuFuncNode func, MuCallConv callconv, MuConstNode cookie);

    /// Create CFG
    
    // Create basic block and add it to function version "fv".
    MuBBNode        (*new_bb        )(MuCtx *ctx, MuFuncVerNode fv);

    // Basic block parameters and instruction results are appended to the basic
    // block or instruction.

    // Create a normal parameter node and add it to the basic block "bb".
    MuNorParamNode  (*new_nor_param )(MuCtx *ctx, MuBBNode bb, MuTypeNode ty);

    // Create an exception parameter node and add it to the basic block "bb".
    MuExcParamNode  (*new_exc_param )(MuCtx *ctx, MuBBNode bb);

    // Create an instruction result. It becomes the next result of "inst".
    MuInstResNode   (*new_inst_res  )(MuCtx *ctx, MuInstNode inst);

    /// Create common clauses for instructions.

    // Create a destination clause and add it to instruction "inst".
    void    (*add_dest      )(MuCtx *ctx, MuInstNode inst, MuDestKind kind, MuBBNode dest, MuVarNode *vars, MuArraySize nvars); /// MUAPIPARSER vars:array:nvars

    // Create a destination clause and add it to instruction "inst".
    void    (*add_keepalives)(MuCtx *ctx, MuInstNode inst, MuLocalVarNode *vars, MuArraySize nvars); /// MUAPIPARSER vars:array:nvars
    
    /// Create instructions. Instructions are appended to the basic block "bb".

    MuInstNode  (*new_binop         )(MuCtx *ctx, MuBBNode bb, MuBinOptr  optr,    MuTypeNode ty,      MuVarNode  opnd1, MuVarNode opnd2);
    MuInstNode  (*new_cmp           )(MuCtx *ctx, MuBBNode bb, MuCmpOptr  optr,    MuTypeNode ty,      MuVarNode  opnd1, MuVarNode opnd2);
    MuInstNode  (*new_conv          )(MuCtx *ctx, MuBBNode bb, MuConvOptr optr,    MuTypeNode from_ty, MuTypeNode to_ty, MuVarNode opnd);
    MuInstNode  (*new_select        )(MuCtx *ctx, MuBBNode bb, MuTypeNode cond_ty, MuTypeNode opnd_ty, MuVarNode  cond,  MuVarNode if_true, MuVarNode if_false);

    MuInstNode  (*new_branch        )(MuCtx *ctx, MuBBNode bb);
    MuInstNode  (*new_branch2       )(MuCtx *ctx, MuBBNode bb, MuVarNode cond);
    MuInstNode  (*new_switch        )(MuCtx *ctx, MuBBNode bb, MuTypeNode opnd_ty, MuVarNode opnd);
    void        (*add_switch_dest   )(MuCtx *ctx, MuInstNode sw, MuConstNode key, MuBBNode dest, MuVarNode *vars, MuArraySize nvars); /// MUAPIPARSER vars:array:nvars

    MuInstNode  (*new_call          )(MuCtx *ctx, MuBBNode bb, MuFuncSigNode sig, MuVarNode callee, MuVarNode *args, MuArraySize nargs); /// MUAPIPARSER args:array:nargs
    MuInstNode  (*new_tailcall      )(MuCtx *ctx, MuBBNode bb, MuFuncSigNode sig, MuVarNode callee, MuVarNode *args, MuArraySize nargs); /// MUAPIPARSER args:array:nargs
    MuInstNode  (*new_ret           )(MuCtx *ctx, MuBBNode bb, MuVarNode *rvs, MuArraySize nrvs); /// MUAPIPARSER rvs:array:nrvs
    MuInstNode  (*new_throw         )(MuCtx *ctx, MuBBNode bb, MuVarNode exc);

    MuInstNode  (*new_extractvalue  )(MuCtx *ctx, MuBBNode bb, MuTypeNode strty, int index,         MuVarNode opnd);
    MuInstNode  (*new_insertvalue   )(MuCtx *ctx, MuBBNode bb, MuTypeNode strty, int index,         MuVarNode opnd, MuVarNode newval);
    MuInstNode  (*new_extractelement)(MuCtx *ctx, MuBBNode bb, MuTypeNode seqty, MuTypeNode indty,  MuVarNode opnd, MuVarNode index);
    MuInstNode  (*new_insertelement )(MuCtx *ctx, MuBBNode bb, MuTypeNode seqty, MuTypeNode indty,  MuVarNode opnd, MuVarNode index, MuVarNode newval);
    MuInstNode  (*new_shufflevector )(MuCtx *ctx, MuBBNode bb, MuTypeNode vecty, MuTypeNode maskty, MuVarNode vec1, MuVarNode vec2,  MuVarNode mask);

    MuInstNode  (*new_new           )(MuCtx *ctx, MuBBNode bb, MuTypeNode allocty);
    MuInstNode  (*new_newhybrid     )(MuCtx *ctx, MuBBNode bb, MuTypeNode allocty, MuTypeNode lenty, MuVarNode length);
    MuInstNode  (*new_alloca        )(MuCtx *ctx, MuBBNode bb, MuTypeNode allocty);
    MuInstNode  (*new_allocahybrid  )(MuCtx *ctx, MuBBNode bb, MuTypeNode allocty, MuTypeNode lenty, MuVarNode length);

    MuInstNode  (*new_getiref       )(MuCtx *ctx, MuBBNode bb, MuTypeNode refty, MuVarNode opnd);
    MuInstNode  (*new_getfieldiref  )(MuCtx *ctx, MuBBNode bb, MuBool is_ptr, MuTypeNode refty, int index, MuVarNode opnd);
    MuInstNode  (*new_getelemiref   )(MuCtx *ctx, MuBBNode bb, MuBool is_ptr, MuTypeNode refty, MuTypeNode indty, MuVarNode opnd, MuVarNode index);
    MuInstNode  (*new_shiftiref     )(MuCtx *ctx, MuBBNode bb, MuBool is_ptr, MuTypeNode refty, MuTypeNode offty, MuVarNode opnd, MuVarNode offset);
    MuInstNode  (*new_getvarpartiref)(MuCtx *ctx, MuBBNode bb, MuBool is_ptr, MuTypeNode refty, MuVarNode opnd);

    MuInstNode  (*new_load          )(MuCtx *ctx, MuBBNode bb, MuBool is_ptr, MuMemOrd ord, MuTypeNode refty,   MuVarNode loc);
    MuInstNode  (*new_store         )(MuCtx *ctx, MuBBNode bb, MuBool is_ptr, MuMemOrd ord, MuTypeNode refty,   MuVarNode loc,     MuVarNode newval);
    MuInstNode  (*new_cmpxchg       )(MuCtx *ctx, MuBBNode bb, MuBool is_ptr, MuBool is_weak,  MuMemOrd ord_succ,  MuMemOrd ord_fail, MuTypeNode refty, MuVarNode loc, MuVarNode expected, MuVarNode desired);
    MuInstNode  (*new_atomicrmw     )(MuCtx *ctx, MuBBNode bb, MuBool is_ptr, MuMemOrd ord, MuAtomicRMWOptr optr, MuTypeNode refTy,  MuVarNode loc,    MuVarNode opnd);
    MuInstNode  (*new_fence         )(MuCtx *ctx, MuBBNode bb, MuMemOrd ord);
    
    MuInstNode  (*new_trap          )(MuCtx *ctx, MuBBNode bb, MuTypeNode *rettys, MuArraySize nrettys); /// MUAPIPARSER rettys:array:nrettys
    MuInstNode  (*new_watchpoint    )(MuCtx *ctx, MuBBNode bb, MuWPID wpid, MuTypeNode *rettys, MuArraySize nrettys); /// MUAPIPARSER rettys:array:nrettys
    MuInstNode  (*new_wpbranch      )(MuCtx *ctx, MuBBNode bb, MuWPID wpid);

    MuInstNode  (*new_ccall         )(MuCtx *ctx, MuBBNode bb, MuCallConv callconv, MuTypeNode callee_ty, MuFuncSigNode sig, MuVarNode callee, MuVarNode *args, MuArraySize nargs); /// MUAPIPARSER args:array:nargs

    MuInstNode  (*new_newthread     )(MuCtx *ctx, MuBBNode bb, MuVarNode stack, MuVarNode threadlocal);
    MuInstNode  (*new_swapstack_ret )(MuCtx *ctx, MuBBNode bb, MuVarNode swappee, MuTypeNode *ret_tys, MuArraySize nret_tys); /// MUAPIPARSER ret_tys:array:nret_tys
    MuInstNode  (*new_swapstack_kill)(MuCtx *ctx, MuBBNode bb, MuVarNode swappee);

    void        (*set_newstack_pass_values)(MuCtx *ctx, MuInstNode inst, MuTypeNode *tys, MuVarNode *vars, MuArraySize nvars); /// MUAPIPARSER tys:array:nvars;vars:array:nvars
    void        (*set_newstack_throw_exc  )(MuCtx *ctx, MuInstNode inst, MuVarNode exc);

    MuInstNode  (*new_comminst      )(MuCtx *ctx, MuBBNode bb, MuCommInst opcode,
                                        MuFlag        *flags, MuArraySize nflags,
                                        MuTypeNode    *tys,   MuArraySize ntys,
                                        MuFuncSigNode *sigs,  MuArraySize nsigs,
                                        MuVarNode     *args,  MuArraySize nargs);
                 /// MUAPIPARSER flags:array:nflags;tys:array:ntys;sigs:array:nsigs;args:array:nargs
};

// Common instruction opcodes
/// SCRIPT: GENERATED COMMINSTS BEGIN
#define MU_CI_UVM_NEW_STACK               ((MuCommInst)0x201)
#define MU_CI_UVM_KILL_STACK              ((MuCommInst)0x202)
#define MU_CI_UVM_THREAD_EXIT             ((MuCommInst)0x203)
#define MU_CI_UVM_CURRENT_STACK           ((MuCommInst)0x204)
#define MU_CI_UVM_SET_THREADLOCAL         ((MuCommInst)0x205)
#define MU_CI_UVM_GET_THREADLOCAL         ((MuCommInst)0x206)
#define MU_CI_UVM_TR64_IS_FP              ((MuCommInst)0x211)
#define MU_CI_UVM_TR64_IS_INT             ((MuCommInst)0x212)
#define MU_CI_UVM_TR64_IS_REF             ((MuCommInst)0x213)
#define MU_CI_UVM_TR64_FROM_FP            ((MuCommInst)0x214)
#define MU_CI_UVM_TR64_FROM_INT           ((MuCommInst)0x215)
#define MU_CI_UVM_TR64_FROM_REF           ((MuCommInst)0x216)
#define MU_CI_UVM_TR64_TO_FP              ((MuCommInst)0x217)
#define MU_CI_UVM_TR64_TO_INT             ((MuCommInst)0x218)
#define MU_CI_UVM_TR64_TO_REF             ((MuCommInst)0x219)
#define MU_CI_UVM_TR64_TO_TAG             ((MuCommInst)0x21A)
#define MU_CI_UVM_FUTEX_WAIT              ((MuCommInst)0x220)
#define MU_CI_UVM_FUTEX_WAIT_TIMEOUT      ((MuCommInst)0x221)
#define MU_CI_UVM_FUTEX_WAKE              ((MuCommInst)0x222)
#define MU_CI_UVM_FUTEX_CMP_REQUEUE       ((MuCommInst)0x223)
#define MU_CI_UVM_KILL_DEPENDENCY         ((MuCommInst)0x230)
#define MU_CI_UVM_NATIVE_PIN              ((MuCommInst)0x240)
#define MU_CI_UVM_NATIVE_UNPIN            ((MuCommInst)0x241)
#define MU_CI_UVM_NATIVE_EXPOSE           ((MuCommInst)0x242)
#define MU_CI_UVM_NATIVE_UNEXPOSE         ((MuCommInst)0x243)
#define MU_CI_UVM_NATIVE_GET_COOKIE       ((MuCommInst)0x244)
#define MU_CI_UVM_META_ID_OF              ((MuCommInst)0x250)
#define MU_CI_UVM_META_NAME_OF            ((MuCommInst)0x251)
#define MU_CI_UVM_META_LOAD_BUNDLE        ((MuCommInst)0x252)
#define MU_CI_UVM_META_LOAD_HAIL          ((MuCommInst)0x253)
#define MU_CI_UVM_META_NEW_CURSOR         ((MuCommInst)0x254)
#define MU_CI_UVM_META_NEXT_FRAME         ((MuCommInst)0x255)
#define MU_CI_UVM_META_COPY_CURSOR        ((MuCommInst)0x256)
#define MU_CI_UVM_META_CLOSE_CURSOR       ((MuCommInst)0x257)
#define MU_CI_UVM_META_CUR_FUNC           ((MuCommInst)0x258)
#define MU_CI_UVM_META_CUR_FUNC_VER       ((MuCommInst)0x259)
#define MU_CI_UVM_META_CUR_INST           ((MuCommInst)0x25A)
#define MU_CI_UVM_META_DUMP_KEEPALIVES    ((MuCommInst)0x25B)
#define MU_CI_UVM_META_POP_FRAMES_TO      ((MuCommInst)0x25C)
#define MU_CI_UVM_META_PUSH_FRAME         ((MuCommInst)0x25D)
#define MU_CI_UVM_META_ENABLE_WATCHPOINT  ((MuCommInst)0x25E)
#define MU_CI_UVM_META_DISABLE_WATCHPOINT ((MuCommInst)0x25F)
#define MU_CI_UVM_META_SET_TRAP_HANDLER   ((MuCommInst)0x260)
/// SCRIPT: GENERATED COMMINSTS END

#ifdef __cplusplus
}
#endif

#endif // __MUAPI_H__
