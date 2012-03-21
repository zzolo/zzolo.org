<?php

/**
 * @file
 * Functions to handle the NodeSquirrel backup destination.
 */

define('NODESQUIRREL_SECRET_KEY_PATTERN', '/^[0-9a-f]{32}\:[0-9a-f]{32}$/');

/**
 * A destination for sending database backups to the NodeSquirel backup service.
 *
 * @ingroup backup_migrate_destinations
 */
class backup_migrate_destination_nodesquirrel extends backup_migrate_destination {
  var $supported_ops = array('scheduled backup', 'manual backup', 'restore', 'list files', 'configure', 'delete');
  var $cache_files = TRUE;
  // Don't generate a metadata file as NodeSquirrel can save metadata natively.
  var $save_metadata = FALSE;

  /**
   * Get the destination name. Provide a default.
   */
  function get_name() {
    if (empty($this->name)) {
      return t('NodeSquirrel');
    }
    return $this->name;
  }

  /**
   * Save to the NodeSquirrel destination.
   */
  function save_file($file, $settings) {
    if ($destination = $this->_get_destination()) {
      srand((double)microtime()*1000000);
      $boundary = '---------------------------'. substr(md5(rand(0,32000)),0,10);

      $filename = $file->filename();
      $filesize = filesize($file->filepath());
      $ticket = $this->_xmlrpc('backups.getUploadTicket', array($destination, $filename, $filesize, $file->file_info));

      if ($ticket) {
        $data = '';

        foreach ((array)$ticket['params'] as $key => $value) {
          $data .="--$boundary\r\n";
          $data .= "Content-Disposition: form-data; name=\"".$key."\"\r\n";
          $data .= "\r\n".$value."\r\n";
          $data .="--$boundary\r\n";
        }
  
        // Add the file to the post payload.
        $data .="--$boundary\r\n";
        $data .= "Content-Disposition: form-data; name=\"file\"; filename=\"". $file->filename() ."\"\r\n";
        $data .= "Content-Type: application/octet-stream;\r\n";
        $data .= "\r\n".(file_get_contents($file->filepath()))."\r\n";
        $data .="--$boundary--\r\n";
    
        $headers = array(
          'Content-type' => "multipart/form-data, boundary=$boundary",
          'User-Agent' => 'NodeSquirrel Client/1.x (+http://www.nodesquirrel.com) (Drupal '. VERSION .'; Backup and Migrate 2.x)',
        );
  
        $url = $ticket['url'];
  
        // If the ticket requires authentication add our username/password to the url.
        if (!empty($ticket['auth']) && $ticket['auth'] = 'basic') {
          $parts = parse_url($ticket['url']);
          list($parts['user'], $parts['pass']) = $this->get_user_pass();
          $url = $this->glue_url($parts, FALSE);
        }

        $out = drupal_http_request($url, array('headers' => $headers, 'method' => 'POST', 'data' => $data));

        if ($out->code == 200) {
          // Confirm the upload.
          $confirm = $this->_xmlrpc('backups.confirmUpload', array($destination, $filename, $filesize));

          if ($confirm['success']) {
            return $file;
          }
          else {
            _backup_migrate_message('The backup file never made it to the NodeSquirrel backup server. There may have been a network problem. Please try again later');
          }
        }
        else {
          $error = !empty($out->headers['x-bams-error']) ? $out->headers['x-bams-error'] : $out->error;
          _backup_migrate_message('The server returned the following error: %err', array('%err' => $error), 'error');
        }
      }
      else if (!xmlrpc_error()) {
        _backup_migrate_message('The server refused the backup but did not specify why. Maybe the server is down.');
      }
    }
    return NULL;
  }

  /**
   * Load from the NodeSquirrel destination.
   */
  function load_file($file_id) {
    if ($destination = $this->_get_destination()) {
      backup_migrate_include('files');
      $file = new backup_file(array('filename' => $file_id));

      $ticket = $this->_xmlrpc('backups.getDownloadTicket', array($destination, $file_id));

      if ($ticket && $url = $ticket['url']) {
        // If the ticket requires authentication add our username/password to the url.
        if (!empty($ticket['auth']) && $ticket['auth'] = 'basic') {
          $parts = parse_url($ticket['url']);
          $parts['user'] = @$this->dest_url['user'];
          $parts['pass'] = @$this->dest_url['pass'];
          $url = $this->glue_url($parts, FALSE);
        }
  
        $out = drupal_http_request($url);

        if ($out->code == 200) {
          file_put_contents($file->filepath(), $out->data);
          return $file;
        }
        else {
          $error = !empty($out->headers['x-bams-error']) ? $out->headers['x-bams-error'] : $out->error;
          _backup_migrate_message('The server returned the following error: %err', array('%err' => $error), 'error');
        }
      }
    }
    return NULL;
  }

  /**
   * Delete from the NodeSquirrel destination.
   */
  function delete_file($file_id) {
    if ($destination = $this->_get_destination()) {
      $result = $this->_xmlrpc('backups.deleteFile', array($destination, $file_id));
    }
  }

  /**
   * List the files in the remote destination.
   */
  function _list_files() {
    $files = array();
    backup_migrate_include('files');

    if ($destination = $this->_get_destination()) {
      $file_list = $this->_xmlrpc('backups.listFiles', array($destination));
      foreach ((array)$file_list as $file) {
        $files[$file['filename']] = new backup_file($file);
      }
    }

    return $files;
  }

  /**
   * List the files in the remote destination.
   */
  function check_limits() {
    $limits = $this->_xmlrpc('backups.getLimits', array($this->_get_destination()));
    return $limits;
  }

  /**
   * List the files in the remote destination.
   */
  function confirm_destination() {
    backup_migrate_include('files');

    $list = $this->_xmlrpc('backups.checkDestination', array($this->_get_destination()));

    return $list;
  }

  /**
   * Get the form for the settings for this destination.
   */
  function edit_form() {
    $form = parent::edit_form();

    $form['settings'] = array('#tree' => TRUE);
    $activate_link = l('nodesquirrel.com/activate', variable_get('nodesquirrel_activate_url', 'http://manage.nodesquirrel.com/activate'), array('query' => array('url' => url('', array('absolute' => TRUE)), 'email' => variable_get('site_mail', ''), 'configure' => url($_GET['q'], array('absolute' => TRUE)))));

    // Retrieve the key from the settings or get it from the get string if this is an auto-config action.
    $key = $this->settings('secret_key');
    if (!empty($_GET['key']) && preg_match(NODESQUIRREL_SECRET_KEY_PATTERN, $_GET['key'])) {
      $key = $_GET['key'];
    }

    $form['settings']['secret_key'] = array(
      '#type' => 'textfield',
      '#title' => t('Secret Key'),
      '#default_value' => $key,
    );
    $form['settings']['location'] = array('#type' => 'value', '#value' => '');

    $form['settings']['secret_key_help'] = array(
      '#type' => 'item',
      '#title' => t('Need a Secret Key?'),
      '#markup' => t('Visit !nodesquirrel.', array('!nodesquirrel' => $activate_link)),
    );

    return $form;
  }



  /**
   * Submit the configuration form. Glue the url together and add the old password back if a new one was not specified.
   */
  function edit_form_validate($form, &$form_state) {
    $key = trim($form_state['values']['settings']['secret_key']);
    if ($key) {
      if (!preg_match(NODESQUIRREL_SECRET_KEY_PATTERN, $key)) {
        form_set_error('secret_key', 'The secret key you entered is not the right format. Please make sure you copy it exactly.');
        return;
      }
      $this->settings['secret_key'] = check_plain($key);

      $limits = $this->check_limits();

      if (!$limits) {
        $err = xmlrpc_error();
        if (!empty($err->code) && $err->code == '401') {
          form_set_error('user', 'Could not login to server. Please check that your secret key is correct.');
        }
        else {
          form_set_error('', 'There was an error retrieving the specified site. Please check that your secret key is correct.');
        }
      }
    }
  }

  /**
   * Submit the configuration form. Glue the url together and add the old password back if a new one was not specified.
   */
  function edit_form_submit($form, &$form_state) {
    $form_state['values']['secret_key'] = check_plain($form_state['values']['settings']['secret_key']);
    parent::edit_form_submit($form, $form_state);
  }

  /**
   * Get the destination id or warn the user that it has not been set.
   */
  function _get_destination($warn = TRUE) {
    list($id, $key) = $this->get_user_pass();
    return $id;
  }

  /**
   * Get the destination id or warn the user that it has not been set.
   */
  function _get_private_key($warn = TRUE) {
    list($id, $key) = $this->get_user_pass();
    return $key;
  }

  function get_user_pass() {
    return explode(':', trim($this->settings('secret_key')));
  }

  function get_display_location() {
    return t('NodeSquirrel.com');
  }

  function add_scheme($url) {
    return 'http://' . $url;
  }

  /**
   * Get the form for the settings for this destination.
   */
  function _xmlrpc($method, $args = array()) {
    // Retrieve the severs or read them from a stored variable.
    $servers = $this->_get_endpoints();

    // Do the actual call.
    return $this->__xmlrpc($method, $args, $servers);
  }

  /**
   * Get the form for the settings for this destination.
   */
  function __xmlrpc($method, $args, $servers, $retry = 3) {
    if ($servers && --$retry > 0) {
      // Add the key authentication arguments if we can.
      if ($this->_sign_request($args)) {  
        $url = reset($servers);
        // Try each available server in order.
        while ($url) {
  
          $url = $this->add_scheme($url);

          $out = xmlrpc($url, array($method => $args));
  
          // Check for errors.
          $err = xmlrpc_error();
          if ($err && $err->is_error) {
            switch ($err->code) {
              case '500':
              case '503':
              case '404':
                // Some sort of server error. Try the next one.
                $url = next($servers);

                // If we're at the end of the line then try refetching the urls
                if (!$url) {
                  $servers = $this->_get_endpoints(TRUE, $retry);
                  return $this->__xmlrpc($method, $args, $servers, $retry);
                }
              break;
              case '300':
                // 'Multiple Choices' means that the existing server list needs to be refreshed.
                $servers = $this->_get_endpoints(TRUE, $retry);
                return $this->__xmlrpc($method, $args, $servers, $retry);
              break;
              case '401':
              case '403':
                // Authentication failed.
                _backup_migrate_message('Couldn\'t log in to NodeSquirrel. The server error was: %err', array('%err' => $err->message), 'error');
                return FALSE;
              break;
              default:
                // Some sort of client error. Don't try the next server because it'll probably say the same thing.
                _backup_migrate_message('The server returned the following error: %err', array('%err' => $err->message), 'error');
                return FALSE;
              break;
            }
          }
          // No error, return the result.
          else {
            return $out;
          }
        }
      }
    }
  }

  /** 
   * Genrate a hash with a given secret key, timestamp and random value.
   */
  function _get_hash($time, $nonce) {
    if ($private_key = $this->_get_private_key()) {
      $message = $time . ':' . $nonce . ':' . $private_key;
      // Use HMAC-SHA1 to authenticate the call.
      $hash = base64_encode(
        pack('H*', sha1((str_pad($private_key, 64, chr(0x00)) ^ (str_repeat(chr(0x5c), 64))) .
        pack('H*', sha1((str_pad($private_key, 64, chr(0x00)) ^ (str_repeat(chr(0x36), 64))) .
        $message))))
      );
      return $hash;
    }
    _backup_migrate_message('You must enter a valid private key to use NodeSquirrel.', array(), 'error');
    return FALSE;
  }

  /** 
   * Genrate a hash with a given secret key, timestamp and random value.
   */
  function _sign_request(&$args) {
    $nonce  = md5(mt_rand());
    $time   = time();
    $hash   = $this->_get_hash($time, $nonce); 
    if ($hash) {
      array_unshift($args, $nonce);
      array_unshift($args, $time);
      array_unshift($args, $hash);
      return TRUE;
    }
    else {
      return FALSE;
    }
  }

  /**
   * Retrieve the list of servers.
   */
  function _get_endpoints($refresh = FALSE, $retry = 3) {
    $servers = (array)variable_get('nodesquirrel_endpoint_urls', array());

    // No servers saved or a force refreshr required.
    if ($refresh || empty($servers)) {
      $servers = array_unique(array_merge($servers, variable_get('nodesquirrel_default_endpoint_urls', array('api.nodesquirrel.com/services/xmlrpc'))));
      // Call the get endpoints method but use the default or previous servers to avoid infinite loops.
      $new_servers = $this->__xmlrpc('backups.getEndpoints', array($this->_get_destination(), 'xmlrpc'), $servers, $retry);
      if ($new_servers) {
        variable_set('nodesquirrel_endpoint_urls', $new_servers);
        $servers = $new_servers;
      }
    }
    return $servers;
  }
}

