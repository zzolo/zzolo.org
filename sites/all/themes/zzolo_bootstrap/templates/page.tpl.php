<?php

/**
 * @file
 * Default theme implementation to display a single Drupal page.
 *
 * Available variables:
 *
 * General utility variables:
 * - $base_path: The base URL path of the Drupal installation. At the very
 *   least, this will always default to /.
 * - $directory: The directory the template is located in, e.g. modules/system
 *   or themes/bartik.
 * - $is_front: TRUE if the current page is the front page.
 * - $logged_in: TRUE if the user is registered and signed in.
 * - $is_admin: TRUE if the user has permission to access administration pages.
 *
 * Site identity:
 * - $front_page: The URL of the front page. Use this instead of $base_path,
 *   when linking to the front page. This includes the language domain or
 *   prefix.
 * - $logo: The path to the logo image, as defined in theme configuration.
 * - $site_name: The name of the site, empty when display has been disabled
 *   in theme settings.
 * - $site_slogan: The slogan of the site, empty when display has been disabled
 *   in theme settings.
 *
 * Navigation:
 * - $main_menu (array): An array containing the Main menu links for the
 *   site, if they have been configured.
 * - $secondary_menu (array): An array containing the Secondary menu links for
 *   the site, if they have been configured.
 * - $breadcrumb: The breadcrumb trail for the current page.
 *
 * Page content (in order of occurrence in the default page.tpl.php):
 * - $title_prefix (array): An array containing additional output populated by
 *   modules, intended to be displayed in front of the main title tag that
 *   appears in the template.
 * - $title: The page title, for use in the actual HTML content.
 * - $title_suffix (array): An array containing additional output populated by
 *   modules, intended to be displayed after the main title tag that appears in
 *   the template.
 * - $messages: HTML for status and error messages. Should be displayed
 *   prominently.
 * - $tabs (array): Tabs linking to any sub-pages beneath the current page
 *   (e.g., the view and edit tabs when displaying a node).
 * - $action_links (array): Actions local to the page, such as 'Add menu' on the
 *   menu administration interface.
 * - $feed_icons: A string of all feed icons for the current page.
 * - $node: The node object, if there is an automatically-loaded node
 *   associated with the page, and the node ID is the second argument
 *   in the page's path (e.g. node/12345 and node/12345/revisions, but not
 *   comment/reply/12345).
 *
 * Regions:
 * - $page['help']: Dynamic help text, mostly for admin pages.
 * - $page['highlighted']: Items for the highlighted content region.
 * - $page['content']: The main content of the current page.
 * - $page['sidebar_first']: Items for the first sidebar.
 * - $page['sidebar_second']: Items for the second sidebar.
 * - $page['header']: Items for the header region.
 * - $page['footer']: Items for the footer region.
 *
 * @see template_preprocess()
 * @see template_preprocess_page()
 * @see template_process()
 */
?>
<div class="topbar">
  <div class="fill">
    <div class="container<?php if ($fluid): ?>-fluid<?php endif; ?>">
      <?php if ($logo): ?>
        <a href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home" id="logo">
          <img src="<?php print $logo; ?>" alt="<?php print t('Home'); ?>" />
        </a>
      <?php endif; ?>
      
      <?php if ($site_name || $site_slogan): ?>
        <div id="name-and-slogan">
          <?php if ($site_name): ?>
            <?php if ($title): ?>
              <div id="site-name"><strong>
                <a class="brand" href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home"><span><?php print $site_name; ?></span></a>
              </strong></div>
            <?php else: /* Use h1 when the content title is empty */ ?>
              <h1 id="site-name">
                <a class="brand" href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home"><span><?php print $site_name; ?></span></a>
              </h1>
            <?php endif; ?>
          <?php endif; ?>
        </div>
      <?php endif; ?>
      
      <?php if ($main_menu || $secondary_menu): ?>
        <div id="navigation">
          <div class="section">
            <?php print theme('links__system_main_menu', array('links' => $main_menu, 'attributes' => array('id' => 'main-menu', 'class' => array('nav', 'links', 'inline', 'clearfix')))); ?>
          </div>
        </div>
      <?php endif; ?>
      
      <?php print render($page['header']); ?>
      
    </div>
  </div>
</div>

<?php 
  // If there is fullscreen, then only render that.
?>
<?php if ($page['fullscreen']): ?>
  <div class="container<?php if ($fluid): ?>-fluid<?php endif; ?> fullscreen">
      <div class="section">
        <?php print render($page['fullscreen']); ?>
      </div>

      <?php if ($is_front): ?>
        <div class="welcome-message">
          <p><strong>Welcome</strong> to <a href="<?php print $front_page; ?>" class="welcome-site-link">zzolo.org</a>, a place to <a href="<?php print url('explore'); ?>" class="welcome-site-link">explore</a> the life of Alan Palazzolo.  This site is a laboratory, so please excuse any messes, or feel free to <a href="<?php print url('contact'); ?>" class="welcome-site-link">express</a> your complaints.</p>
        </div>
      <?php endif; ?>
  </div>
<?php else: ?>
  <?php if ($page['hero']): ?>
    <div class="container<?php if ($fluid): ?>-fluid<?php endif; ?>">
      <div id="hero" class="hero-unit">
        <div class="section">
          <?php print render($page['hero']); ?>
        </div>
      </div>
    </div>
  <?php endif; ?>
  
  <div class="container<?php if ($fluid): ?>-fluid<?php endif; ?> main-content-container">
    <?php print $messages; ?>
    
    <div id="main-wrapper">
      <div id="main" class="clearfix">
        <div class="section">
          <?php if ($page['highlighted']): ?><div id="highlighted"><?php print render($page['highlighted']); ?></div><?php endif; ?>
          <a id="main-content"></a>
          <?php print render($title_prefix); ?>
          
          <div class="feed-icons-container pull-right">
            <?php print $feed_icons; ?>
          </div>
          
          <?php if ($flippy_next): ?>
            <div class="flippy flippy-next"><?php print $flippy_next; ?></div>
          <?php endif; ?>
          <?php if ($node_type): ?>
            <div class="label label-node-type"><?php print $node_type; ?></div>
          <?php endif; ?>
          <?php if ($flippy_prev): ?>
            <div class="flippy flippy-prev"><?php print $flippy_prev; ?></div>
          <?php endif; ?>
          
          <?php if ($title): ?>
            <h1 class="title" id="page-title">
              <?php print $title; ?>
              <?php if ($sub_title): ?>
                <small><?php print $sub_title; ?></small>
              <?php endif; ?>
            </h1>
          <?php endif; ?>
          <?php print render($title_suffix); ?>
          
          <?php if ($tabs): ?>
            <div class="tabs-container clearfix">
              <?php print render($tabs); ?>
            </div>
          <?php endif; ?>
          
          <?php print render($page['help']); ?>
          
          <?php if ($action_links): ?>
            <ul class="action-links">
              <?php print render($action_links); ?>
            </ul>
          <?php endif; ?>
          
          <?php if ($page['masonry']): ?>
            <div class="masonry-container">
              <?php print render($page['masonry']); ?>
            </div>
          <?php endif; ?>
          
          <?php print render($page['content']); ?>
        </div>
      </div>
    </div> 
  </div>
    
  <div id="footer" class="container<?php if ($fluid): ?>-fluid<?php endif; ?>">
    <div class="section">
      <div class="social-links">
        <a href="http://github.com/zzolo" class="svg_github">github</a>
        <a href="http://twitter.com/zzolo" class="svg_twitter">twitter</a>
        <a href="<?php print url('contact'); ?>" class="svg_email">contact</a>
        <a href="http://www.linkedin.com/in/alanpalazzolo" class="svg_linkedin">linkedin</a>
        <a href="http://www.flickr.com/photos/masada4242/" class="svg_flickr">flickr</a>
      </div>
      
      <?php print render($page['footer']); ?>
    </div>
  </div>
<?php endif; ?>
