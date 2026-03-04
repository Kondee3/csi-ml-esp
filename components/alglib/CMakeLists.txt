idf_component_register(
    SRCS "src/alglibinternal.cpp"
         "src/alglibmisc.cpp" 
         "src/ap.cpp"
         "src/dataanalysis.cpp"
         "src/diffequations.cpp"
         "src/fasttransforms.cpp"
         "src/integration.cpp"
         "src/interpolation.cpp"
         "src/kernels_avx2.cpp"
         "src/kernels_fma.cpp"
         "src/kernels_sse2.cpp"
         "src/linalg.cpp"
         "src/optimization.cpp"
         "src/solvers.cpp"
         "src/specialfunctions.cpp"
         "src/statistics.cpp"
    INCLUDE_DIRS "include"
    
    REQUIRES ""
    
)