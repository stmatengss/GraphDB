#if !defined(COMMON_H_)
#define COMMON_H_

/* === Temple === */
uint16_t P_TYPE = 1 << 10;
uint16_t V_TYPE = 2 << 10;
uint16_t E_TYPE = 3 << 10;
/* ============== */

enum ret_status {
    failed,
    succeed,
    error
};

#endif // COMMON_H_
