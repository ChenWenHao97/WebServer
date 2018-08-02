<html>
<body>
<?php
    # 3个用来接收的全局变量：$_POST, $_GET, $_REQUEST
    $a=$_POST["A"];
    $b=$_POST["B"];
    $c=$a+$b;
    echo $a.' + '.$b." = $c";
?>

</body>
</html>