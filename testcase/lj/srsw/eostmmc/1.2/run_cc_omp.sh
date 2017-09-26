prog=../muvttmmclj
args="-t 1.2 -z -2.902929 -x 390 -o 1"

if [ -z $FEASST_INSTALL_DIR_ ]; then
  echo "ERROR: the bash variable FEASST_INSTALL_DIR_ must be set to run:"
  echo $0
  echo "Suggest adding the following to ~/.bash_profile"
  echo "export FEASST_INSTALL_DIR_=\"\$HOME/path/to/feasst\""
  exit 1
fi
$FEASST_INSTALL_DIR_/tools/compile.sh $prog
if [ $? -eq 0 ]; then
  mkdir -p tmp #directory for checkpoint files
  ./$prog $args
else
  echo "ERROR: Compilation failed"
fi
