static PyObject *PyPetscScalar_FromPetscScalar(PetscScalar s)
{
#if defined(PETSC_USE_COMPLEX)
  double a = (double) PetscRealPart(s);
  double b = (double) PetscImaginaryPart(s);
  return PyComplex_FromDoubles(a, b);
#else
  return PyFloat_FromDouble((double)s);
#endif
}

static PetscScalar PyPetscScalar_AsPetscScalar(PyObject *o)
{
#if defined(PETSC_USE_COMPLEX)
  Py_complex cval = PyComplex_AsCComplex(o);
  PetscReal a = (PetscReal) cval.real;
  PetscReal b = (PetscReal) cval.imag;
  return a + b * PETSC_i;
#else
  return (PetscScalar) PyFloat_AsDouble(o);
#endif
}

static PetscReal PyPetscScalar_AsPetscComplexReal(PyObject *o)
{
#if defined(PETSC_HAVE_COMPLEX)
  Py_complex cval = PyComplex_AsCComplex(o);
  PetscReal a = (PetscReal)cval.real;
  return a;
#else
  return (PetscScalar)PyFloat_AsDouble(o);
#endif
}

static PetscReal PyPetscScalar_AsPetscComplexImag(PyObject *o)
{
#if defined(PETSC_HAVE_COMPLEX)
  Py_complex cval = PyComplex_AsCComplex(o);
  PetscReal b = (PetscReal)cval.imag;
  return b;
#else
  return 0.0;
#endif
}

