L = 6;
N = 2*L + 1;
alpha = 1; beta = 0; kappa = 3*L/2;

// initial state [roll; pitch; yaw; bx; by; bz] (° and °/s)
x = [1.15; -0.57; 2.86; 0.57; -0.57; 0.29];
P = diag([11.46, 11.46, 17.19, 0.02, 0.02, 0.02]);

Q = diag([0.5, 0.5, 1, 1e-3, 1e-3, 1e-3]);
R = diag([0.00147,0.00147,0.0033,9]);

W0a = (kappa*alpha^2 - L)/(kappa*alpha^2);
W0c = W0a + 1 - alpha^2 + beta;
Wj = (1 - W0a)/(2*L);
Wa = Wj*ones(N,1); Wc = Wj*ones(N,1);
Wa(1) = W0a; Wc(1) = W0c;

dt = 1;
disp("Vector trạng thái x0 = "); disp(x);
disp("Ma trận hiệp phương sai P0 = "); disp(P);
disp("Trọng số sigma points Wa = "); disp(Wa);
disp("Trọng số sigma points Wc = "); disp(Wc);

x_pred_store = zeros(L,20);
z_meas_store = zeros(3,20);
x_upd_store = zeros(L,20);

// =======================
// 20-step simulated data
// =======================
// acc (g): 20x3
acc_data = [
  0.40  -0.30  0.90;
 -0.50   0.20  1.10;
  0.60  -0.40  0.80;
 -0.30   0.50  0.95;
  0.20  -0.60  1.05;
 -0.40   0.40  0.85;
  0.01  -0.02  0.99;
 -0.03   0.02  1.002;
  0.02   0.01  0.995;
  0.00  -0.01  1.010;
  0.015  0.005 0.980;
 -0.02  -0.03  1.005;
  0.01   0.00  0.995;
  0.00   0.02  1.000;
  0.70   0.60  0.40;
 -0.20  -0.10  0.90;
  0.50  -0.70  0.60;
  0.00   0.00  1.00;
  1.20  -0.90  0.30;
 -0.60   0.80  0.70
];

// gyro (deg/s): 20x3
gyro_data = [
   0.1  0.2   0.3
  -20   15   -5;
   30  -25   40;
  -10   12   -8;
   25  -30   20;
  -35   40  -20;
   10   -8   15;
   -1.5  0.9  0.6;
   -1.1  0.7  0.4;
   -1.2  0.8  0.5;
   -1.0  0.85 0.45;
   -1.3  0.78 0.55;
  -1.25  0.82 0.52;
  -1.22  0.79 0.49;
  -1.18  0.81 0.50;
   45  -30   60;
   -5    3   -2;
   20  -15   10;
    0    0    0;
  -60   50  -40;
   12   -9    8
];

// mag (Gauss): 20x3
mag_data = [
  0.020  -0.010  0.005;
 -0.015   0.020 -0.010;
  0.025  -0.030  0.020;
  0.010   0.000 -0.005;
 -0.020   0.015  0.010;
  0.030  -0.020 -0.010;
  0.003  -0.004  0.001;
  0.0031 -0.0046  0.0012;
  0.0029 -0.0044  0.0013;
  0.0030 -0.0045  0.00128;
  0.00305 -0.00452 0.00128;
  0.00302 -0.00450 0.00130;
  0.00300 -0.00448 0.00125;
  0.00310 -0.00460 0.00120;
  0.050  -0.060  0.020;
  0.0030 -0.0040  0.0010;
 -0.040   0.030 -0.020;
  0.00305 -0.00452 0.00128;
  0.060  -0.050  0.040;
  0.0020 -0.0020  0.0005
];
//////////////////////////////////////////////////////////////////////
function [roll, pitch, yaw] = euler_from_acc_mag(acc, mag)
    //acc:[ax; ay; az] (đơn vị g)
    //mag:[mx; my; mz] (đơn vị gauss)
    
    
    roll  = atan(acc(2), acc(3));                   
    pitch = atan(-acc(1), sqrt(acc(2)^2 + acc(3)^2));
    
    
    Mx = mag(1)*cos(pitch) + mag(3)*sin(pitch);
    My = mag(1)*sin(roll)*sin(pitch) + mag(2)*cos(roll) - mag(3)*sin(roll)*cos(pitch);
    
    yaw = atan(My, Mx);
    
    roll  = roll * 180 / %pi;
    pitch = pitch * 180 / %pi;
    yaw   = yaw * 180 / %pi;
    
    //disp('Roll = %.4f°, Pitch = %.4f°, Yaw = %.4f°\n', roll, pitch, yaw);
end
////////////////////////////////////////////////////////////////
function sigma = generate_sigma_points(x, P)
    sigma = zeros(L, N);
    
    A = chol(P)';
    
    sigma(:,1) = x;
    
    for i = 1:L
        sigma(:, i+1)   = x + alpha*sqrt(kappa) * A(:,i);
        sigma(:, i+1+L) = x - alpha*sqrt(kappa) * A(:,i);
    end
    //disp(alpha*sqrt(kappa));
    //disp("A = "); disp(A);
    //disp("sigma = "); disp(sigma);
endfunction
////////////////////////////////////////////////////////////////

function [x_pred, P_pred, sigma_pred] = ukf_predict(sigma, Wa, Wc, Q, dt, gyro)
    sigma_pred = zeros(L, N);
    
    for j = 1:N
        roll  = sigma(1,j) + (gyro(1) - sigma(4,j))*dt;   // roll + bx*dt
        pitch = sigma(2,j) + (gyro(2) - sigma(5,j))*dt;   // pitch + by*dt
        yaw   = sigma(3,j) + (gyro(3) - sigma(6,j))*dt;   // yaw + bz*dt
        
        bx = sigma(4,j);
        by = sigma(5,j);
        bz = sigma(6,j);
        
        sigma_pred(:,j) = [roll; pitch; yaw; bx; by; bz];
    end
    //disp("sigma_pred = "); disp(sigma_pred);
    x_pred = zeros(L,1);
    for j = 1:N
        x_pred = x_pred + Wa(j)*sigma_pred(:,j);
    end
    //disp("x_pred = "); disp(x_pred);
    P_pred = Q;
    for j = 1:N
        dx = sigma_pred(:,j) - x_pred;
        P_pred = P_pred + Wc(j)*(dx*dx');
    end
    //disp("y_pred = "); disp(P_pred);
endfunction
//////////////////////////////////////////////////////////////////
function z = measurement_function(x)
    // x = [roll; pitch; yaw; bx; by; bz]
    roll  = x(1)* %pi /180;
    pitch = x(2)* %pi /180;
    yaw   = x(3);
    //disp("roll = "); disp(sin(roll));
    //disp("pitch = "); disp(pitch);
    //disp("yaw = "); disp(yaw);
    ax = -sin(pitch);
    ay =  sin(roll) * cos(pitch);
    az =  cos(roll) * cos(pitch);
    
    z = [ax; ay; az; yaw];
endfunction
//////////////////////////////////////////////////////////////////
function [x_upd, P_upd, K] = ukf_update(x_pred, P_pred, sigma_pred, Wa, Wc, R, z_meas)
    m = length(z_meas);
    
    Z = zeros(m, N);
    for j = 1:N
        Z(:,j) = measurement_function(sigma_pred(:,j));
    end
    //disp("sigma_z = "); disp(Z);
    z_pred = zeros(m,1);
    for j = 1:N
        z_pred = z_pred + Wa(j)*Z(:,j);
    end
   // disp("z_pred = "); disp(z_pred);
    S = R;
    for j = 1:N
        dz = Z(:,j) - z_pred;
        S = S + Wc(j)*(dz*dz');
    end
    //disp("S = "); disp(S);
    Cxz = zeros(L,m);
    for j = 1:N
        dx = sigma_pred(:,j) - x_pred;
        dz = Z(:,j) - z_pred;
        Cxz = Cxz + Wc(j)*(dx*dz');
    end
   // disp("Cxz = "); disp(Cxz);
    K = Cxz * inv(S);
    //disp("Kalman gain K1 = "); disp(K);
    x_upd = x_pred + K*(z_meas - z_pred);
    P_upd = P_pred - K*S*K';
    //disp("x1|1 = "); disp(x_upd);
    //disp("P1|1 = "); disp(P_upd);
    
endfunction
//////MAIN//////////////////////
for k = 1:20
    acc = acc_data(k,:)';
    gyro = gyro_data(k,:)';
    mag = mag_data(k,:)';
    
    // 1. Sigma points
    sigma = generate_sigma_points(x, P);
    
    // 2. Predict
    [x_pred, P_pred, sigma_pred] = ukf_predict(sigma, Wa, Wc, Q, dt, gyro);
    
    // 3. Measurement
    [roll_acc, pitch_acc, yaw_mag] = euler_from_acc_mag(acc, mag);
    z_meas = [acc; yaw_mag];
    z_meas_angle = [roll_acc; pitch_acc; yaw_mag];
    // Store
    x_pred_store(:,k) = x_pred;
    z_meas_store(:,k) = z_meas_angle;
    
    // 4. Update
    [x_upd, P_upd, K] = ukf_update(x_pred, P_pred, sigma_pred, Wa, Wc, R, z_meas);
    x_upd_store(:,k) = x_upd;
    
    // Update state for next step
    x = x_upd;
    P = P_upd;
end

// Plot Roll/Pitch/Yaw
scf(0); clf;
t = 1:20;

for i = 1:3
    subplot(3,1,i);
    plot(t, x_pred_store(i,:), 'r-o');
    plot(t, x_upd_store(i,:), 'b-o');
    plot(t, z_meas_store(i,:), 'g--'); xtitle(["Roll (Predicted = red, Updated = blue, Measured = green)","Pitch","Yaw"](i) + " (°)");
end


//legend(["Predicted","Updated","Measured"], "center");

// Plot bias
scf(1); clf;
for i = 1:3
    subplot(3,1,i); plot(t, x_upd_store(i+3,:)); xtitle(["bx","by","bz"](i) + " (°/s)");
end



