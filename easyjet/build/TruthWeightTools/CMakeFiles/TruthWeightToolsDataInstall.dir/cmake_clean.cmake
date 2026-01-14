file(REMOVE_RECURSE
  "../x86_64-el9-gcc14-opt/data/TruthWeightTools/VBF_weights.xml"
  "../x86_64-el9-gcc14-opt/data/TruthWeightTools/VH_weights.xml"
  "../x86_64-el9-gcc14-opt/data/TruthWeightTools/ggF_NNLOPS_weights.XMLOLD"
  "../x86_64-el9-gcc14-opt/data/TruthWeightTools/ggF_NNLOPS_weights.xml"
  "../x86_64-el9-gcc14-opt/data/TruthWeightTools/ttH_weights.xml"
  "genConf/TruthWeightTools.confdb2_part"
  "../x86_64-el9-gcc14-opt/data/TruthWeightTools/VBF_weights.xml"
  "../x86_64-el9-gcc14-opt/data/TruthWeightTools/VH_weights.xml"
  "../x86_64-el9-gcc14-opt/data/TruthWeightTools/ggF_NNLOPS_weights.XMLOLD"
  "../x86_64-el9-gcc14-opt/data/TruthWeightTools/ggF_NNLOPS_weights.xml"
  "../x86_64-el9-gcc14-opt/data/TruthWeightTools/ttH_weights.xml"
  "../x86_64-el9-gcc14-opt/include/TruthWeightTools"
  "../x86_64-el9-gcc14-opt/lib/libTruthWeightTools.confdb"
  "../x86_64-el9-gcc14-opt/python/TruthWeightTools/TruthWeightToolsConf.py"
  "../x86_64-el9-gcc14-opt/python/TruthWeightTools/__init__.py"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/TruthWeightToolsDataInstall.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
