const char* html = R"html(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Control Interface</title>
    <style>
        #mouseControls {
            position: relative;
            width: 8cm;
            height: 6cm;
        }
        .mouseButton {
            position: absolute;
            width: 2cm;
            height: 2cm;
            font-size: 16px;
            text-align: center;
        }
        
        #left { left: 0; top: 2cm; }
        #right { right: 0; top: 2cm; }
        #up { left: 3cm; top: 0; }
        #down { left: 3cm; bottom: 0; }
        #leftClick { left: 2cm; top: 2cm; }
        #rightClick { right: 2cm; top: 2cm; }
    </style>
</head>
<body>
    <div id="mouseControls">
        <button id="left" class="mouseButton"  onmousedown="moveMouse(-1, 0)" onmouseup="stopMouse()" ontouchstart="moveMouse(-1, 0);event.preventDefault()"  ontouchend="stopMouse()" >←</button>
        <button id="right" class="mouseButton" onmousedown="moveMouse(1, 0)"  onmouseup="stopMouse()" ontouchstart="moveMouse(1, 0); event.preventDefault()"  ontouchend="stopMouse()" >→</button>
        <button id="up" class="mouseButton"    onmousedown="moveMouse(0, -1)" onmouseup="stopMouse()" ontouchstart="moveMouse(0, -1);event.preventDefault()"  ontouchend="stopMouse()" >↑</button>
        <button id="down" class="mouseButton"  onmousedown="moveMouse(0, 1)"  onmouseup="stopMouse()" ontouchstart="moveMouse(0, 1); event.preventDefault()"  ontouchend="stopMouse()" >↓</button>
        <button id="leftClick" class="mouseButton" onclick="mouseClick()">L</button>
        <button id="rightClick" class="mouseButton" onclick="mouseClick(true)">R</button>
    </div>
    <br><br>
    <input type="text" id="dataInput" placeholder="Enter ascii text">
    <button onclick="printText()">Print</button>
    <br><br>
    <input type="number" id="keyCodeInput" placeholder="Enter key code">
    <button onmousedown="keyPress()" onmouseup="keyRelease()">Press Key</button>
    <br><br>
    

    <script>
        function printText() {
            const data = document.getElementById('dataInput').value;
            fetch('/kbd_print', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: `data=${encodeURIComponent(data)}`
            })
            .then(response => response.text())
            .then(result => console.log('Success:', result))
            .catch(error => console.error('Error:', error));
        }

        function keyPress() {
            const keyCode = document.getElementById('keyCodeInput').value;
            fetch('/kbd_press', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: `key=${encodeURIComponent(keyCode)}`
            });
        }

        function keyRelease() {
            const keyCode = document.getElementById('keyCodeInput').value;
            fetch('/kbd_release', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: `key=${encodeURIComponent(keyCode)}`
            });
        }

        let moveInterval = null;
        function moveMouse(dx, dy) {
            let x = 0, y = 0;
            moveInterval = setInterval(function() {
                x += dx;
                y += dy;
                if(x > 100) x = 100;
                if(x < -100) x = -100;
                if(y > 100) y = 100;
                if(y < -100) y = -100;
                fetch('/mouse_rel_move', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded',
                    },
                    body: `x=${x}&y=${y}`
                });
            }, 100); // Adjust frequency as needed
        }

        function stopMouse() {
            if (moveInterval) {
                clearInterval(moveInterval);
                moveInterval = null;
            }
        }

        function mouseClick(left = false) {
            const body = left ? 'left=true' : '';
            fetch('/mouse_click', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: body
            });
        }
    </script>
</body>
</html>
)html";