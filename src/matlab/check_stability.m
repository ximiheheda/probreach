function res = check_stability(A,B,C,D,T,Kp,Ki,Kd)

    %A = matrix(A);
    %B = matrix(B);
    %C = matrix(C);
    %D = matrix(D);

    fileID = fopen('poly.txt','w');

    fprintf(fileID, '\nMatrix A\n');
    fprintf(fileID, '%f ', A);
    fprintf(fileID, '\nMatrix B\n');
    fprintf(fileID, '%f ', B);
    fprintf(fileID, '\nMatrix C\n');
    fprintf(fileID, '%f ', C);
    fprintf(fileID, '\nMatrix D\n');
    fprintf(fileID, '%f ', D);

    % computing G, state transition matrix of discrete system
    G = expm(A*T);

    % computing H, integrating approximately
    H = zeros(size(B));
    fprintf(fileID, '\nSize H:\n');
    fprintf(fileID, '%f ', size(H));
    n = 10000;
    t = linspace(0,T,n+1);
    delta = T/n;
    for i=1:n
        %fprintf(fileID, '\nMatrix exp\n');
        %fprintf(fileID, '%f ', expm(A*t(i)));
        %fprintf(fileID, '\n');
        H = H + expm(A*t(i))*B*delta;
    end

    [bp,ap] = ss2tf(G,H,C,D);
    fprintf(fileID, '\nbp:\n');
    fprintf(fileID, '%f ', bp);

    fprintf(fileID, '\nap:\n');
    fprintf(fileID, '%f ', ap);

    % transfer function of the controller
    ac = [1 -1 0]; % numerator
    % bc = [a b c] denumerator should be of this form
    % we can translate these a b c in terms of Kp Ki Kd
    % for stability we just need to comute

    a = Kp+Ki+Kd;
    b = -Kp-2*Kd;
    c = Kd;

    bc = [a b c];

    poly = conv(ac,ap) + conv(bc,bp);

    fprintf(fileID, '\nParameters: %f %f %f\n', Kp, Ki, Kd);
    fprintf(fileID, '\nabc: %f %f %f\n', a, b, c);
    fprintf(fileID, '\nDiscretisation: %f\n', T);
    fprintf(fileID, '\nMatrix G\n');
    fprintf(fileID, '%f ', G);
    fprintf(fileID, '\nMatrix H\n');
    fprintf(fileID, '%f ', H);
    fprintf(fileID, '\nCharacteristic polynomial:\n');
    fprintf(fileID, '%f ', poly);

    % below is Jury test for the obtained characteristic polynomial
    n = length(poly);

    % condition 1
    if(abs(poly(end)) >= poly(1))
        %disp('Condition 1 is violated');
        res = 1;
        return;
    end

    % condition 2
    if(polyval(poly, 1) <= 0)
        %disp('Condition 2 is violated');
        res = 1;
        return;
    end

    % condition 3
    % even order
    if(mod(n-1, 2) == 0)
        if(polyval(poly, -1) <= 0)
            %disp('Condition 3 is violated');
            res = 1;
            return;
        end
    % odd order
    else
        if(polyval(poly, -1) >= 0)
            %disp('Condition 3 is violated');
            res = 1;
            return;
        end
    end

    % contructing a Jury stability table
    J = zeros(2*(n-1)-3,n);

    % constructing the first two rows of the table
    J(1,:) = fliplr(poly);
    J(2,:) = poly;
    i = 3;
    bound = n;
    % constructing the rest of the table
    % and checking condition 4
    while i <= 2*(n-1)-3
        for k=1:bound-1
            J(i,bound-k)=J(i-1,bound)*J(i-1,k+1)-J(i-1,bound-k)*J(i-1,1);
        end
        if (abs(J(i,1)) <= abs(J(i,bound-1)))
            %disp('Condition 4 is violated for i=');
            %disp(i);
            res = 1;
            return;
        end
        i = i+1;
        if(i > 2*(n-1)-3)
            break;
        end
        J(i,1:bound-1) = fliplr(J(i-1,1:bound-1));
        i=i+1;    
        bound=bound-1;
    end

    res = 0;    

    fprintf(fileID, 'Result = %f ', res);
    fclose(fileID);


end

%================End of the code====================================



