for i=1:8
    generation(i*4, i*2, 100, 1000);
end

disp('first stage over');

format long g
for i=11:20
    generation(24, 16, i*5, 1000);
end

disp('second stage over');