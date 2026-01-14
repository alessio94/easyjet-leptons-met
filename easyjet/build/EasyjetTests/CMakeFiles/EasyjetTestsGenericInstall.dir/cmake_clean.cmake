file(REMOVE_RECURSE
  "../x86_64-el9-gcc14-opt/tab-complete.bash"
  "../x86_64-el9-gcc14-opt/./tab-complete.bash"
  "../x86_64-el9-gcc14-opt/bin/check-lfs"
  "../x86_64-el9-gcc14-opt/bin/easyjet-clang-tidy"
  "../x86_64-el9-gcc14-opt/bin/easyjet-cppcheck"
  "../x86_64-el9-gcc14-opt/bin/easyjet-test"
  "../x86_64-el9-gcc14-opt/bin/easyjet-validate"
  "../x86_64-el9-gcc14-opt/bin/metadata-check"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/EasyjetTestsGenericInstall.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
