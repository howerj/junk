#include <stdio.h>
#include <math.h>

double square(double x) { return x * x; }
double cube(double x) { return x * x * x; }
double sine(double x) { return sin(x); }
double cosine(double x) { return cos(x); }

/* Compute the integral of f() within the interval [a,b] */
double integral(double f(double x), double a, double b, int n)
{
    int i;
    double sum = 0;
    double dt = (b - a) / n;
    for (i = 0;  i < n;  ++i) {
        sum += f(a + (i + 0.5) * dt);
    }
    return sum * dt;
}

int main(void)
{
    printf("%g\n", integral(square, 0, 1, 100));
    printf("%g\n", integral(cube, 0, 1, 100));
    printf("%g\n", integral(sine, 0, 1, 100));
    printf("%g\n", integral(cosine, 0, 1, 100));
    return 0;
}

