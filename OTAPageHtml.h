// HTML for OTA Firmware Upgrade page
String otaPageHtml =
  "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
  "<link href=\"https://docs.espressif.com/favicon.ico\" rel=\"icon\" type=\"image/x-icon\">"
  "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jquery/3.4.1/jquery.min.js\"></script>"
  "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/4.3.1/css/bootstrap.min.css\">"
  "<section id=\"cover\" class=\"min-vh-100 position-relative d-flex align-items-center\">"
    "<div class=\"vw-100 position-relative\">"
      "<div class=\"container\">"
        "<div class=\"row px-4\">"
          "<form id=\"fwForm\" method=\"POST\" action=\"#\" enctype=\"multipart/form-data\" class=\"col-lg-6 col-md-8 col-sm-10 mx-auto p-4\">"
            "<h3 class=\"mb-4 text-center font-weight-bold\">ESP32 OTA FW UPGRADE</h3>"
            "<div class=\"custom-file mb-3\">"
              "<input type=\"file\" accept=\".bin\" class=\"custom-file-input\" id=\"fileInput\" name=\"update\" onchange=\"sub(this.value)\">"
              "<label style=\"padding-right: 80px;\" class=\"custom-file-label text-truncate\" for=\"fileInput\" id=\"fileInputLabel\">Select file...</label>"
            "</div>"
            "<div class=\"progress mb-3\">"
              "<div class=\"progress-bar progress-bar-striped\" style=\"width: 100%;\" id=\"progress\">Progress</div>"
            "</div>"
            "<div class=\"text-center\">"
              "<button type=\"submit\" class=\"btn btn-custom font-weight-bold\">Upload</button>"
            "</div>"
          "</form>"
        "</div>"
      "</div>"
    "</div>"
  "</section>"
  "<script>"
    "function sub(val) {"
      "var lblTxt = 'Select file...';"
      "if(val.length > 0) {"
        "var fName = val.split('\\\\');"
        "lblTxt = fName[fName.length - 1];"
      "}"
      "$('#fileInputLabel').html(lblTxt);"
    "}"
    "$('#fwForm').on('submit', function(e) {"
      "e.preventDefault();"
      "if ($('#fileInput').val().length === 0) return false;"
      "var data = new FormData($('#fwForm')[0]);"
      "$.ajax({"
        "url: '/upgrade',"
        "type: 'POST',"
        "data: data,"
        "contentType: false,"
        "processData: false,"
        "xhr: function () {"
          "var xhr = new window.XMLHttpRequest();"
          "xhr.upload.addEventListener('progress', function (evt) {"
            "if (evt.lengthComputable) {"
              "var pct = Math.round((evt.loaded / evt.total)* 100);"
              "$('#progress').html('').css('width', pct + '%');"
              "if (pct > 10) {"
                "$('#progress').css({'text-align':'right', 'padding-right':'0.5rem'}).html(pct + '%');"
              "}"
            "}"
          "}, false);"
          "return xhr;"
        "},"
        "success: function (d, s) {"
          "$('#progress').css({'text-align':'center', 'padding-right':'0'}).html('Success! Rebooting...');"
          "setTimeout(function() { location.reload(); }, 5 * 1000); /* in ms */"
        "},"
        "error: function (a, b, c) { }"
      "});"
      "return false;"
    "});"
  "</script>"
  "<style>"
    "#cover { background-color: #181818; }"
    "form { color: #e8e8e8; background-color: #282828; border: 1px solid #580818; box-shadow: 0 0 8px 0 #580818; border-radius: 0.5rem; }"
    ".custom-file-label { color: #e8e8e8; background-color: #282828; border: 1px solid #484848; }"
    ".custom-file-label:after { color: #e8e8e8; background-color: #580818; }"
    ".custom-file-input:focus~.custom-file-label { box-shadow: none; border-color: #484848; }"
    ".progress { background-color: #282828; border: 1px solid #484848; height: 1.25rem; }"
    ".progress-bar { color: #e8e8e8; background-color: #580818; }"
    ".btn-custom { color: #e8e8e8; background-color: #580818; border-color: #484848; }"
    ".btn-custom:hover, .btn-custom:active { color: #e8e8e8; background-color: #351417; border-color: #484848; }"
    ".btn-custom:focus { box-shadow: none; }"
  "</style>";
