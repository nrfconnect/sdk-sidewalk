wget https://raw.githubusercontent.com/git/git/master/contrib/completion/git-prompt.sh -O ~/.git-prompt.sh

PS1_bash='\u@\h \[\033[32m\]\w\[\033[33m\]$(__git_ps1 " (%s)")\[\033[00m\$ '
echo "" >> ~/.bashrc
echo "source ~/.git-prompt.sh" >> ~/.bashrc
echo "export PS1='${PS1_bash}'" >> ~/.bashrc
