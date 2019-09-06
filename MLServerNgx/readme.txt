Ubuntu 下编译：
    sudo mount -t vboxsf -o uid=1000,gid=1000 zikiplayer /home/henry/zikiplayer
    Fixing your virtualbox shared folder symlink error: https://ahtik.com/fixing-your-virtualbox-shared-folder-symlink-error/
        VBoxManage setextradata YOURVMNAME VBoxInternal2/SharedFoldersEnableSymlinksCreate/YOURSHAREFOLDERNAME 1

    sudo apt-get install systemtap-sdt-dev
    ubuntu: 
    * Configure before build: third-parties/openresty/config-for-ubuntu.sh
    * build.sh

开发文档:
* Openresty template 语法
  * https://github.com/bungle/lua-resty-template
* Openresty 参考手册
  * https://openresty-reference.readthedocs.io/en/latest/Lua_Nginx_API/#ngxhttp_time
* OpenResty 最佳实践 
  * https://moonbingbing.gitbooks.io/openresty-best-practices/content/
* Ajax axios
  * https://github.com/axios/axios

测试：
    curl -i -X POST http://search.crintsoft.com/searchlyrics.htm -H "Content-Type: text/xml"   --data-binary "@/Users/hongyongxiao/zikiplayer/ML-Protocol-Lua-Lib/test/request_0.bin"
    scp /Users/hongyongxiao/zikiplayer/MLServerNgx/product/nginx ubuntu@mlserver:~/
    sudo cp nginx /usr/local/openresty/nginx/sbin/nginx
