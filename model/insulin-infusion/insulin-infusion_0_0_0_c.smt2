(set-logic QF_NRA_ODE)
; declaring functions and their bounds
(declare-fun I () Real)
(declare-fun I_0_0 () Real)
(declare-fun I_0_t () Real)
(assert (>= I_0_0  0))
(assert (>= I_0_t  0))
(assert (<= I_0_0  10000))
(assert (<= I_0_t  10000))
(declare-fun Q1 () Real)
(declare-fun Q1_0_0 () Real)
(declare-fun Q1_0_t () Real)
(assert (>= Q1_0_0  0))
(assert (>= Q1_0_t  0))
(assert (<= Q1_0_0  10000))
(assert (<= Q1_0_t  10000))
(declare-fun Q2 () Real)
(declare-fun Q2_0_0 () Real)
(declare-fun Q2_0_t () Real)
(assert (>= Q2_0_0  0))
(assert (>= Q2_0_t  0))
(assert (<= Q2_0_0  10000))
(assert (<= Q2_0_t  10000))
(declare-fun S1 () Real)
(declare-fun S1_0_0 () Real)
(declare-fun S1_0_t () Real)
(assert (>= S1_0_0  0))
(assert (>= S1_0_t  0))
(assert (<= S1_0_0  10000))
(assert (<= S1_0_t  10000))
(declare-fun S2 () Real)
(declare-fun S2_0_0 () Real)
(declare-fun S2_0_t () Real)
(assert (>= S2_0_0  0))
(assert (>= S2_0_t  0))
(assert (<= S2_0_0  10000))
(assert (<= S2_0_t  10000))
(declare-fun tau () Real)
(declare-fun tau_0_0 () Real)
(declare-fun tau_0_t () Real)
(assert (>= tau_0_0  0))
(assert (>= tau_0_t  0))
(assert (<= tau_0_0  500))
(assert (<= tau_0_t  500))
(declare-fun x1 () Real)
(declare-fun x1_0_0 () Real)
(declare-fun x1_0_t () Real)
(assert (>= x1_0_0  0))
(assert (>= x1_0_t  0))
(assert (<= x1_0_0  10000))
(assert (<= x1_0_t  10000))
(declare-fun x2 () Real)
(declare-fun x2_0_0 () Real)
(declare-fun x2_0_t () Real)
(assert (>= x2_0_0  0))
(assert (>= x2_0_t  0))
(assert (<= x2_0_0  10000))
(assert (<= x2_0_t  10000))
(declare-fun x3 () Real)
(declare-fun x3_0_0 () Real)
(declare-fun x3_0_t () Real)
(assert (>= x3_0_0  0))
(assert (>= x3_0_t  0))
(assert (<= x3_0_0  10000))
(assert (<= x3_0_t  10000))
; assigning the values of the sample
; declaring time variable and integration bounds
(declare-fun time () Real)
(assert (>= time 0.0))
(assert (<= time 108.9789721312039))
; defining odes
(define-ode flow_1 ((= d/dt[I] (-(/ S2(* 55(* 0.12 100)))(* 0.138 I)))
(= d/dt[Q1] (+(+(-(+(-(-(/(/(*(* 0.0097 100)(/ Q1(* 0.16 100)))(+(/ Q1(* 0.16 100)) 1)) 0.85))(* x1 Q1))(* 0.066 Q2)) 0)(*(* 0.0161 100)(- 1 x3)))(/(* 8 180) 1000)))
(= d/dt[Q2] (-(* x1 Q1)(*(+ 0.066 x2) Q2)))
(= d/dt[S1] (- 0.0(/ S1 55)))
(= d/dt[S2] (/(- S1 S2) 55))
(= d/dt[tau]  1)
(= d/dt[x1] (+(*(- 0.006) x1)(* 0.0034 I)))
(= d/dt[x2] (+(*(- 0.06) x2)(* 0.056 I)))
(= d/dt[x3] (+(*(- 0.03) x3)(* 0.024 I)))
))
; defining initial condition
(assert (and (>= I_0_0 0.03) (<= I_0_0 0.03)))
(assert (and (>= Q1_0_0 63.99999999999999) (<= Q1_0_0 64.00000000000001)))
(assert (and (>= Q2_0_0 39.99999999999999) (<= Q2_0_0 40.00000000000001)))
(assert (and (>= S1_0_0 4.199999999999999) (<= S1_0_0 4.200000000000001)))
(assert (and (>= S2_0_0 4) (<= S2_0_0 4.000000000000001)))
(assert (and (>= tau_0_0 -4.940656458412465e-324) (<= tau_0_0 4.940656458412465e-324)))
(assert (and (>= x1_0_0 0.03) (<= x1_0_0 0.03)))
(assert (and (>= x2_0_0 0.04499999999999999) (<= x2_0_0 0.04500000000000001)))
(assert (and (>= x3_0_0 0.05) (<= x3_0_0 0.05000000000000001)))
; defining the integral
(assert (= [I_0_t Q1_0_t Q2_0_t S1_0_t S2_0_t tau_0_t x1_0_t x2_0_t x3_0_t ] (integral 0.0 time [I_0_0 Q1_0_0 Q2_0_0 S1_0_0 S2_0_0 tau_0_0 x1_0_0 x2_0_0 x3_0_0 ] flow_1)))
; defining invariants negations
(assert (or
))
(check-sat)
(exit)
