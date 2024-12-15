#ifndef MAT_H
#define MAT_H

#include <bits/stdc++.h>
using namespace std;

class matrix
{
public:
    int row, col;
    double **mat;

    matrix(int row, int col, bool identity = 1)
    {
        this->row = row;
        this->col = col;
        mat = new double *[row];
        for (int i = 0; i < row; i++)
        {
            mat[i] = new double[col];
            for (int j = 0; j < col; j++)
            {
                mat[i][j] = 0 + identity * (i == j);
            }
        }
    }

    matrix(const matrix &m)
    {
        row = m.row;
        col = m.col;
        mat = new double *[row];
        for (int i = 0; i < row; i++)
        {
            mat[i] = new double[col];
            for (int j = 0; j < col; j++)
            {
                mat[i][j] = m.mat[i][j];
            }
        }
    }

    ~matrix()
    {
        for (int i = 0; i < row; i++)
        {
            delete[] mat[i];
        }
        delete[] mat;
    }

    matrix operator+(const matrix &m) const
    {
        matrix result(row, col);
        if (row != m.row || col != m.col)
        {
            cout << "Error: Matrix dimensions do not match" << endl;
            return result;
        }

        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < col; j++)
                result.mat[i][j] = mat[i][j] + m.mat[i][j];
        }

        return result;
    }

    matrix operator-(const matrix &m) const
    {
        matrix result(row, col);
        if (row != m.row || col != m.col)
        {
            cout << "Error: Matrix dimensions do not match" << endl;
            return result;
        }

        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < col; j++)
                result.mat[i][j] = mat[i][j] - m.mat[i][j];
        }

        return result;
    }

    matrix operator*(const matrix &m) const
    {
        matrix result(row, m.col);
        if (col != m.row)
        {
            cout << "Error: Matrix dimensions do not match" << endl;
            return result;
        }

        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < m.col; j++)
            {
                result.mat[i][j] = 0;
                for (int k = 0; k < col; k++)
                {
                    result.mat[i][j] += mat[i][k] * m.mat[k][j];
                }
            }
        }

        return result;
    }

    matrix operator*(double val) const
    {
        matrix result(row, col);
        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < col; j++)
                result.mat[i][j] = mat[i][j] * val;
        }

        return result;
    }

    matrix operator/(double val) const
    {
        matrix result(row, col);
        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < col; j++)
                result.mat[i][j] = mat[i][j] / val;
        }

        return result;
    }

    matrix operator=(const matrix &m)
    {
        if (this == &m)
            return *this;

        for (int i = 0; i < row; i++)
            delete[] mat[i];
        delete[] mat;

        row = m.row;
        col = m.col;
        mat = new double *[row];
        for (int i = 0; i < row; i++)
        {
            mat[i] = new double[col];
            for (int j = 0; j < col; j++)
                mat[i][j] = m.mat[i][j];
        }

        return *this;
    }

    friend ostream &operator<<(ostream &out, const matrix &m)
    {
        for (int i = 0; i < m.row; i++)
        {
            for (int j = 0; j < m.col; j++)
                out << fixed << setprecision(7) << m.mat[i][j] << " ";
            out << endl;
        }

        return out;
    }
};

#endif
