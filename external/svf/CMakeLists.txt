add_library( svflib STATIC IMPORTED GLOBAL )

set_target_properties( svflib PROPERTIES
    IMPORTED_LOCATION ${SVF_INSTALL_DIR}/lib/libSvf.a
    INTERFACE_INCLUDE_DIRECTORIES "${SVF_INSTALL_DIR}/include;${SVF_INSTALL_DIR}/include/svf"
)
