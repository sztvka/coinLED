
jQuery(document).ready(function($){
    var gecko = "https://api.coingecko.com/api/v3/coins/list";

  $('#table_id').DataTable( {
    "ajax": {
        "url": gecko,
        "dataSrc": ""
    },
    "columns": [
        { "data": "id" },
        { "data": "name" }
    ]
} );  
});