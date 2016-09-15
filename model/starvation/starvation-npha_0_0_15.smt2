(set-logic QF_NRA_ODE)
(declare-fun F () Real)
(declare-fun F_0_0 () Real)
(declare-fun F_0_t () Real)
(assert (>= F_0_0  0))
(assert (>= F_0_t  0))
(assert (<= F_0_0  100))
(assert (<= F_0_t  100))
(declare-fun K () Real)
(declare-fun K_0_0 () Real)
(declare-fun K_0_t () Real)
(assert (>= K_0_0  0))
(assert (>= K_0_t  0))
(assert (<= K_0_0  100))
(assert (<= K_0_t  100))
(declare-fun M () Real)
(declare-fun M_0_0 () Real)
(declare-fun M_0_t () Real)
(assert (>= M_0_0  0))
(assert (>= M_0_t  0))
(assert (<= M_0_0  100))
(assert (<= M_0_t  100))
(declare-fun k () Real)
(declare-fun k_0_0 () Real)
(declare-fun k_0_t () Real)
(declare-fun t_g () Real)
(declare-fun t_g_0_0 () Real)
(declare-fun t_g_0_t () Real)
(assert (>= t_g_0_0  20))
(assert (>= t_g_0_t  20))
(assert (<= t_g_0_0  25))
(assert (<= t_g_0_t  25))
(declare-fun tau () Real)
(declare-fun tau_0_0 () Real)
(declare-fun tau_0_t () Real)
(assert (>= tau_0_0  0))
(assert (>= tau_0_t  0))
(assert (<= tau_0_0  100))
(assert (<= tau_0_t  100))
(declare-fun time_0 () Real)
(assert (>= time_0  0))
(assert (<= time_0  100))
(define-ode flow_1 ((= d/dt[F] (* F(-(/(- 0.013)(+ 1 K))(*(/ 1 7777.8)(+(/(+ 772.3(* k 30.4))(+ F M)) k)))))(= d/dt[K] (-(/(*(* 0.9 0.013) F)(+ 1 K)) 0.075))(= d/dt[M] (*(/(- M) 1400)(+(/(+ 772.3(* k 30.4))(+ F M)) k)))(= d/dt[k]  0)(= d/dt[t_g]  0)(= d/dt[tau]  1)))
(assert (and 
(or ((and(= tau_0_0 0)(= F_0_0 25)(= M_0_0 43.6)(= K_0_0 0.02))))
(>= k_0_0 11.3795)
(<= k_0_0 11.3795)
(>= t_g_0_0 24.0364)
(<= t_g_0_0 24.0364)
(= [F_0_t K_0_t M_0_t k_0_t t_g_0_t tau_0_t ] (integral 0.0 time_0 [F_0_0 K_0_0 M_0_0 k_0_0 t_g_0_0 tau_0_0 ] flow_1))
(or ((and(= tau_0_t t_g_0_t)(<= M_0_t(* 0.6 43.6)))))))
(check-sat)
(exit)
