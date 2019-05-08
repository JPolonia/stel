file = fopen('counters', 'r');

min = fscanf(file, '%f', 1);
max = fscanf(file, '%f', 1);
int = fscanf(file, '%d', 1);

n = fscanf(file, '%d');
fclose(file);

v = (max-min)/int;
a = (min:v:max-v);

n = n./sum(n);

bar(a, n);
