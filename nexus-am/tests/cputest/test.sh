#bin/bash!

for FILE in `ls ./tests`
do
    echo "\033[34m [ test ${FILE} ] \033[0m"
    (make ARCH=$ISA-nemu ALL=${FILE%\.*} run)
done