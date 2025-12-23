WEB-INF是java的WEB应用的安全目录，此外如果想在页面访问WEB-INF应用里面的文件，必须要通过web.xml进行相应的映射才能访问。  
其中敏感目录举例：
```http
/WEB-INF/web.xml：
# Web应用程序配置文件，描述了 servlet 和其他的应用组件配置及命名规则

/WEB-INF/classes/：
# 含了站点所有用的 class 文件，包括 servlet class 和非servlet class，他们不能包含在.jar文件中

/WEB-INF/lib/：
# 存放web应用需要的各种JAR文件，放置仅在这个应用中要求使用的jar文件,如数据库驱动jar文件

/WEB-INF/src/：
# 源码目录，按照包名结构放置各个java文件

/WEB-INF/database.properties：
# 数据库配置文件
```
