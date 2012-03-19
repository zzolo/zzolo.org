/**
 * @file
 * Javascript file for Typekit module.
 */

(function($) {

  /**
   * Drupal behavior to handle adding Typekit script, either as https or http.
   */
  Drupal.behaviors.typekit = {
    attach: function(context, settings) {
      var tkJsHost = (('https:' == document.location.protocol) ? 'https://' : 'http://');
      $.getScript(tkJsHost + 'use.typekit.com/' + settings.typekitKey + '.js', function(data, textStatus) {
        try {
          Typekit.load();
        }
        catch (e) {};
      });
    }
  };

})(jQuery);
