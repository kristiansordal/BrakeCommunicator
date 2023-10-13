#pragma once
#include "mesh.h"
#include <cmath>

template <typename T> int Mesh<T>::size() { return mesh_rows_ * mesh_cols_; }
template <typename T> int Mesh<T>::m_dim() { return mesh_rows_; }
template <typename T> int Mesh<T>::n_dim() { return mesh_cols_; }
