# Script to analyze sliding windowed file data

library(tidyverse)

clusters <- c(1,3,7,7)

for( i in c(1,2,3,4)){
  data <- read_csv(paste0('~/bytewise-stats/examples/20220802_rustsj_',i,'.dump.csv'))

  data_kmeans <- data %>% 
    select(-byte_offset) %>% 
    kmeans(clusters[[i]])

  kmeans_results <- tibble(byte_offset=data$byte_offset,
                           cluster=data_kmeans$cluster) %>%
    left_join(data,by=c(byte_offset='byte_offset'))
  
  kmeans_results %>% 
    pivot_longer(cols=!byte_offset&!cluster,names_to='byte',values_to='count') %>%
    mutate(byte=as.numeric(byte))%>%
    ggplot(aes(x=byte_offset,y=byte,fill=hsv(cluster/10,1,if_else(count>10,1,count/10))))+
    geom_raster(show.legend = FALSE) +
    scale_fill_identity() +
    scale_y_continuous(breaks=c(0,32,127,255)) +
    scale_x_continuous(breaks=seq(0,2^18,2^15),limits=c(0,2^16))
  
  ggsave(paste0('~/bytewise-stats/examples/20220802_rustsj_',i,'.png'))
}