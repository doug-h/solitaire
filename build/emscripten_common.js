var Module = {
    print: (function() {})(),
    printErr: function(text) {
        if (arguments.length > 1)
            text = Array.prototype.slice.call(arguments).join(" ");
        if (0) {
            dump(text + "\n");
        }
    },
  onRuntimeInitialized: function() {
        var e = document.getElementById('loading-div');
        e.style.visibility = 'hidden';

        var canvas = document.getElementById("canvas");
        var cont = document.getElementById("resizable-container"); 
        cont.addEventListener("mousemove", function() {
          canvas.style.width  = Math.round(Math.max(cont.style.width, cont.style.min_width));
          canvas.style.height = Math.round(Math.max(cont.style.height, cont.style.min_height));
          window.dispatchEvent(new Event('resize'));
        }, false);

    },
    canvas: (function() {
        var canvas = document.getElementById("canvas");
        return canvas;
    })(),
};

