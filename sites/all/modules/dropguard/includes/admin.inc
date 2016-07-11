<?php

/**
 * @file
 * Contains administrative forms to manage DropGuard settings.
 */

/**
 * Main settings form of DropGuard module.
 */
function dropguard_settings_form($form) {

  $form['info'] = array(
    '#type' => 'fieldset',
    '#title' => t('Connect to DropGuard'),
    // TODO: Add more informative description?
    '#description' => t('All required information about how to connect your web site you can get at DropGuard site.'),
    '#collapsible' => TRUE,
  );

  $form['info']['dropguard_client_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Client ID'),
    '#description' => t('Copy Client ID here from DropGuard service. It is available during creation of a new project or editing existing one.'),
    '#required' => TRUE,
    '#maxlength' => 10,
    '#size' => 10,
    '#element_validate' => array('element_validate_integer_positive'),
    '#default_value' => variable_get('dropguard_client_id'),
  );

  // We call it "Access token", but it is actually a base64-encoded public key
  // for openssl encryption/decryption. Just to avoid misunderstandings from
  // client point of view, we want to provide the most user-friendly name.
  $form['info']['dropguard_openssl_public_key'] = array(
    '#type' => 'textarea',
    '#title' => t('Access Token'),
    '#description' => t('Copy Access Token to this textarea from DropGuard service. It is available during creation of a new project or editing of existing one.'),
    '#required' => TRUE,
    '#size' => 6,
    '#default_value' => variable_get('dropguard_openssl_public_key'),
  );

  // Make sure that PHP OpenSSL library is enabled.
  // Print warning message and disable possibility to input date otherwise.
  if (!extension_loaded('openssl')) {
    $form['info']['#description'] .= '<br/><span style="color:red;">' . t('<a href="@url">PHP OpenSSL extension</a> is missing on your server. DropGuard needs it for secure transfer of information from your web site.', array('@url' => 'http://php.net/manual/en/book.openssl.php')) . '</span>';
    $form['info']['dropguard_client_id']['#disabled'] = TRUE;
    $form['info']['dropguard_openssl_public_key']['#disabled'] = TRUE;
  }

  return system_settings_form($form);
}

/**
 * Validate main settings form of DropGuard module.
 */
function dropguard_settings_form_validate($form, &$form_state) {
  $values = &$form_state['values'];

  // Trim possible whitespaces.
  $values['dropguard_client_id'] = trim($values['dropguard_client_id']);
  $values['dropguard_openssl_public_key'] = trim($values['dropguard_openssl_public_key']);
}
